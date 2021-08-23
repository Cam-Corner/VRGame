// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRCharacterComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Saving/ControlSettingsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/ExtraMaths.h"
#include "DrawDebugHelpers.h"
#include "Networking/NetworkingHelpers.h"
#include "Player/VRCharacter.h"

DEFINE_LOG_CATEGORY(LogAVRCharacterComponent);

// Sets default values for this component's properties
UVRCharacterComponent::UVRCharacterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//bReplicates = true;
	// ...

	ServerSideCap = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ServerSideCap"));
}

// Called when the game starts
void UVRCharacterComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentMovementMode = EMovementModes::EMM_Walk;

	LoadSettings();
	CurrentProxyMove.bStillMoving = false;
	ErrorProxyMove.bStillMoving = false;
	bNewSimProxyUpdate = false;
	// ...

	if(GetOwner())
		ServerSideCap->SetupAttachment(GetOwner()->GetRootComponent());

	if (VRCharacterCapsule)
	{
		ServerSideCap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		ServerSideCap->SetWorldLocation(VRCharacterCapsule->GetComponentLocation());
	}
}

// Called every frame
void UVRCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	NetworkedMovement(DeltaTime);

	UpdateMultiTime -= DeltaTime;

	//ScaleCollisionWithPlayer();
	//MoveCollisionToHMD();
	//HandleMovement(DeltaTime);
	//SmoothRotation(DeltaTime);
	//CheckToSeeIfCameraIsInsideObject();
	//ApplyGravity(DeltaTime);

	//Server_SendMove(VRCharacterCamera->GetComponentLocation());
}

void UVRCharacterComponent::ResetToStartLocation()
{
	if (!VRCharacterCapsule)
		return;

	ServerSideCap->SetWorldLocation(VRCharacterCapsule->GetComponentLocation());
	
	ActorsCapHalfHeight = VRCharacterCapsule->GetScaledCapsuleHalfHeight();
	OldCapHalfHeight = ActorsCapHalfHeight;
}

void UVRCharacterComponent::XYRecenter()
{
	if (!VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterCapsule || !ComponentOwner)
		return;

	FVector MoveOffset;
	MoveOffset.X = VRCharacterCapsule->GetComponentLocation().X - VRCharacterCamera->GetComponentLocation().X;
	MoveOffset.Y = VRCharacterCapsule->GetComponentLocation().Y - VRCharacterCamera->GetComponentLocation().Y;
	MoveOffset.Z = 0;
	
	FVector NewLocation = VRCharacterCameraOrigin->GetComponentLocation() + MoveOffset;
	VRCharacterCameraOrigin->SetWorldLocation(NewLocation);
}

void UVRCharacterComponent::ZRecenter()
{
	if (!VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterCapsule || !ComponentOwner)
		return;

	FVector MoveOffset;
	MoveOffset.X = 0;
	MoveOffset.Y = 0;
	MoveOffset.Z = (VRCharacterCapsule->GetComponentLocation().Z - VRCharacterCapsule->GetScaledCapsuleHalfHeight())
		 - VRCharacterCameraOrigin->GetComponentLocation().Z;

	VRCharacterCameraOrigin->AddWorldOffset(MoveOffset);
}

void UVRCharacterComponent::SetXYMovementDirection(FVector2D Dir)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	GEngine->AddOnScreenDebugMessage(20, 0.1f, FColor::Blue, Dir.ToString(), true);

	float ThumbstickDeadzone = 0.5f;

	if (Dir.X > ThumbstickDeadzone || Dir.X < -ThumbstickDeadzone || Dir.Y > ThumbstickDeadzone || Dir.Y < -ThumbstickDeadzone)
	{
		FVector FinalDir = FVector(0, 0, 0);

		if (Dir.Y > ThumbstickDeadzone || Dir.Y < -ThumbstickDeadzone)
			FinalDir += VRCharacterCapsule->GetForwardVector() * Dir.Y;

		if (Dir.X > ThumbstickDeadzone || Dir.X < -ThumbstickDeadzone)
			FinalDir += VRCharacterCapsule->GetRightVector() * Dir.X;

		switch (CurrentMovementMode)
		{
		case EMovementModes::EMM_SlowWalk:
			XYMovement = FinalDir * SlowWalkMovementSpeed;
			break;
		case EMovementModes::EMM_Walk:
			XYMovement = FinalDir * WalkMovementSpeed;
			break;
		case EMovementModes::EMM_Run:
			XYMovement = FinalDir * RunMovementSpeed;
			break;
		default:
			break;
		}
	}
	else
	{
		XYMovement = FVector{ 0, 0, 0 };
		VelocityDirection = FVector{0, 0, 0};
		ForwardVelDirection = FVector(0, 0, 0);
		RightVelDirection = FVector(0, 0, 0);
	}
}

void UVRCharacterComponent::SaveSettings()
{
	if (UControlSettingsSaveGame* SG =
		Cast<UControlSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UControlSettingsSaveGame::StaticClass())))
	{
		SG->bUsingSmoothTurning = bSmoothTurning;
		SG->SmoothTurningSensitivity = SmoothTurningSensitivity;
		SG->SnapTurningAmount = SnapTurningAmount;

		if (UGameplayStatics::SaveGameToSlot(SG, SaveSlotName, UserIndex))
		{

		}
	}
}

void UVRCharacterComponent::LoadSettings()
{
	if (UControlSettingsSaveGame* LG =
		Cast<UControlSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex)))
	{
		bSmoothTurning = LG->bUsingSmoothTurning;
		SmoothTurningSensitivity = LG->SmoothTurningSensitivity;
		SnapTurningAmount = LG->SnapTurningAmount;
	}
}

void UVRCharacterComponent::NetworkedMovement(float DeltaTime)
{
	bool bLocallyControlled = false;
	ENetRole Role = GetOwnerRole();
	if (AVRCharacter* VRC = Cast<AVRCharacter>(GetOwner()))
	{
		if (VRC->IsLocallyControlled())
		{			
			LocalMovement(DeltaTime);
			MoveCollisionToHMD(DeltaTime);
			ApplyGravity(DeltaTime);
			SmoothRotation(DeltaTime);
			CheckToSeeIfCameraIsInsideObject();		
			ScaleCollisionWithPlayerNetworked(true);
			CheckHMDDistanceFromCollision();
			
			if (VRCharacterCamera)
			{
				Server_SetHMDLocation(VRCharacterCamera->GetComponentLocation());
			}
			
			LastCapLocation = VRCharacterCapsule->GetComponentLocation();
			bLocallyControlled = true;
		}		
	} 
	
	if (Role >= ROLE_Authority)
	{
		//AuthorativeMovement(DeltaTime);

		//if(VRCharacterCapsule)
			//SyncedServerLocation = VRCharacterCapsule->GetComponentLocation();
	} 
	
	if (Role >= ROLE_SimulatedProxy && !bLocallyControlled)
	{
		if (VRCharacterCamera)
			VRCharacterCamera->SetWorldLocation(SyncedHMDLocation);

		//MoveCollisionToHMD(DeltaTime);
		CheckHMDDistanceFromCollision();
		CheckToSeeIfCameraIsInsideObject();
		SimulatedProxyMovement(DeltaTime);
		
		if (ActorsCapHalfHeight != OldCapHalfHeight)
		{
			OldCapHalfHeight = ActorsCapHalfHeight;
			ScaleCollisionWithPlayerNetworked(false);
		}
			
	}
}

void UVRCharacterComponent::LocalMovement(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	/*Set the forward rotation to the Yaw of the camera*/
	FRotator NewRotation = VRCharacterCapsule->GetComponentRotation();
	NewRotation.Yaw = VRCharacterCamera->GetComponentRotation().Yaw;
	VRCharacterCapsule->SetWorldRotation(NewRotation);

	FVector OldCharacterCapsuleLoc = VRCharacterCapsule->GetComponentLocation();
	OldCharacterCapsuleLoc.Z = 0;
	
	if (!XYMovement.IsZero())
	{
		bSendStoppedMove = false;
		/*FPlayerMove Move;
		Move.StartLocation = VRCharacterCapsule->GetComponentLocation();*/
		FVector Dir = XYMovement;
		Dir.Normalize();
		//float Distance = XYMovement.Size();
		MovePlayerCapsuleWithServerSynced(Dir, WalkMovementSpeed * DeltaTime, VRCharacterCapsule, false, true, DeltaTime);
		
		//GEngine->AddOnScreenDebugMessage(25, 5.0f, FColor::Red,
			//NewCapLocation.ToString(), true);

		/*Move.DeltaTime = DeltaTime;
		Move.Dir = Dir;
		
		Move.EndLocation = VRCharacterCapsule->GetComponentLocation();

		if (GetOwnerRole() >= ROLE_Authority)
		{
			FClientSide_Prediction NewMove;
			NewMove.LastServerWorldDelta = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();
			NewMove.LastServerLocation = Move.EndLocation;
			NewMove.Dir = Move.Dir;
			NewMove.bStillMoving = true;

			GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Blue, "Host sending move", true);
				NetMulticast_SendMove(NewMove.Dir, NewMove.LastServerLocation, NewMove.LastServerWorldDelta, NewMove.bStillMoving);

			if (ServerSideCap)
				ServerSideCap->SetWorldLocation(Move.EndLocation);
		}
		else
		{
			Server_SendMove(Move.Dir, Move.DeltaTime, Move.EndLocation);
		}*/
	}
	else
	{
		if (!bSendStoppedMove)
		{
			/*FPlayerMove Move;
			Move.DeltaTime = DeltaTime;
			Move.Dir = FVector::ZeroVector;
			Move.StartLocation = VRCharacterCapsule->GetComponentLocation();
			Move.EndLocation = VRCharacterCapsule->GetComponentLocation();
			bSendStoppedMove = true;

			if (GetOwnerRole() >= ROLE_Authority)
			{
				FClientSide_Prediction NewMove;
				NewMove.LastServerWorldDelta = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();
				NewMove.LastServerLocation = Move.EndLocation;
				NewMove.Dir = Move.Dir;
				NewMove.bStillMoving = false;

				GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Blue, "Host sending move", true);
				NetMulticast_SendMove(NewMove.Dir, NewMove.LastServerLocation, NewMove.LastServerWorldDelta, NewMove.bStillMoving);
			}
			else
			{
				Server_SendMove(Move.Dir, Move.DeltaTime, Move.EndLocation);
			}*/

			bSendStoppedMove = true;
			MovePlayerCapsuleWithServerSynced(FVector::ZeroVector, 0, VRCharacterCapsule, false, true, DeltaTime);
		}
	}
	

	FVector NewCapsuleLocation = VRCharacterCapsule->GetComponentLocation();
	NewCapsuleLocation.Z = 0;

	FVector OriginOffset = NewCapsuleLocation - OldCharacterCapsuleLoc;

	VRCharacterCameraOrigin->AddWorldOffset(OriginOffset);

}

void UVRCharacterComponent::SimulatedProxyMovement(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	/*GEngine->AddOnScreenDebugMessage(50, 5.0f, FColor::Red,
		CurrentProxyMove.ToString(), true);
	GEngine->AddOnScreenDebugMessage(51, 5.0f, FColor::Red,
		ErrorProxyMove.ToString(), true);*/

	if (bNewSimProxyUpdate)
	{
		GEngine->AddOnScreenDebugMessage(27, 0.1f, FColor::Red,
			"Moving Sim Proxy: " + ErrorProxyMove.ToString(), true);
		
		float OffsetAmount = 0;

		if (CurrentProxyMove.bIsGravity)
			OffsetAmount = GravitySpeed * DeltaTime;
		else
			OffsetAmount = WalkMovementSpeed * DeltaTime;

		MovePlayerCapsule(ErrorProxyMove.Dir, OffsetAmount, VRCharacterCapsule, false, true);

		FVector Location = VRCharacterCapsule->GetComponentLocation();

		float Dist = FVector::Distance(Location, ErrorProxyMove.LastServerLocation);

		if (Dist <= WalkMovementSpeed * DeltaTime)
		{
			bNewSimProxyUpdate = false;
		}
		else if (Dist > 150)
			VRCharacterCapsule->SetWorldLocation(ErrorProxyMove.LastServerLocation);
	}
	else
	{
		if (CurrentProxyMove.bStillMoving)
		{
			GEngine->AddOnScreenDebugMessage(27, 0.1f, FColor::Red,
				"Moving Sim Proxy: " + ErrorProxyMove.ToString(), true);

			MovePlayerCapsule(CurrentProxyMove.Dir, WalkMovementSpeed * DeltaTime,
				VRCharacterCapsule, false, true);
		}
	}
}

void UVRCharacterComponent::AuthorativeMovement(float DeltaTime)
{
	if (!ServerSideCap)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "ServerSideCap NULL", true);
		return;
	}
		
	if (ClientsMoves.Num() > 0)
	{
		FPlayerMove Move = ClientsMoves[0];
		
		float OffsetAmount = 0;

		if (Move.bIsGravity)
			OffsetAmount = GravitySpeed * Move.DeltaTime;
		else
			OffsetAmount = WalkMovementSpeed * Move.DeltaTime;

		MovePlayerCapsule(Move.Dir, OffsetAmount,
			ServerSideCap, false, true);

		SyncedServerLocation = ServerSideCap->GetComponentLocation();

		float Dist = FVector::Distance(SyncedServerLocation, Move.EndLocation);

		if (Dist > 0.5f)
		{
			Client_SetLocation(SyncedServerLocation);
			GEngine->AddOnScreenDebugMessage(55, 5.0f, FColor::Blue, "A clients move was invalid, reseting location!", true);
		}

		FClientSide_Prediction NewMove;
		NewMove.LastServerWorldDelta = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();
		NewMove.LastServerLocation = SyncedServerLocation;
		NewMove.Dir = Move.Dir;
		
		if (Move.Dir == FVector::ZeroVector)
			NewMove.bStillMoving = false;
		else
			NewMove.bStillMoving = true;

		//GEngine->AddOnScreenDebugMessage(25, 1.0f, FColor::Orange, "Sending Clients new move", true);
		NetMulticast_SendMove(NewMove.Dir, NewMove.LastServerLocation, NewMove.LastServerWorldDelta, 
			NewMove.bStillMoving, Move.bIsGravity);

		ClientsMoves.RemoveAt(0);
	}
}

void UVRCharacterComponent::WorkOutSimulatedProxyError()
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FVector CurrentLocaton = VRCharacterCapsule->GetComponentLocation();
	float TimeDifference = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds()
		- CurrentProxyMove.LastServerWorldDelta;
	
	/*GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
		FString::SanitizeFloat(TimeDifference), true);*/
	
	ErrorProxyMove.LastServerLocation = CurrentProxyMove.LastServerLocation
		+ (CurrentProxyMove.Dir * (TimeDifference * WalkMovementSpeed));

	ErrorProxyMove.Dir = ErrorProxyMove.LastServerLocation - CurrentLocaton;
	ErrorProxyMove.Dir.Normalize();

	bNewSimProxyUpdate = true;
}

void UVRCharacterComponent::HandleMovement(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	/*Set the forward rotation to the Yaw of the camera*/
	FRotator NewRotation = VRCharacterCapsule->GetComponentRotation();
	NewRotation.Yaw = VRCharacterCamera->GetComponentRotation().Yaw;
	VRCharacterCapsule->SetWorldRotation(NewRotation);
	
	FVector OldCharacterCapsuleLoc = VRCharacterCapsule->GetComponentLocation();
	OldCharacterCapsuleLoc.Z = 0;
	bool btest = false;
	//Apply the desired movement
	{
		if (!XYMovement.IsZero())
		{
			FVector Dir = XYMovement;
			Dir.Normalize();
			float Distance = XYMovement.Size();
			//MovePlayerCapsule(Dir, Distance * DeltaTime);
			
			if (bUseSweepMovement && btest)
			{
				FHitResult Hit;
				FVector Offset = XYMovement * DeltaTime;
				VRCharacterCapsule->AddWorldOffset(Offset, true, &Hit);
				
				if (Hit.bBlockingHit)
				{
					float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), Hit.ImpactNormal);

					//GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "Slope Angle: " + FString::SanitizeFloat(Angle), true);

					if (Angle > MaxWalkableSlopeAngle)
					{
						FVector Normal2D = Hit.Normal;
						Normal2D.Z = 0;
						Normal2D.Normalize();

						FVector PlaneProjectDir = FVector::VectorPlaneProject(Offset, Normal2D);
						Offset = PlaneProjectDir * (1 - Hit.Time);													
						bool bZRecenter = false;
						
						/* Check to see if their is a slope below us */
						{
							FHitResult FloorHit;
							FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
							float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
							FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 2.5f));
							FCollisionShape SphereTrace;
							FCollisionQueryParams SphereParams;
							SphereParams.AddIgnoredActor(ComponentOwner);

							SphereTrace.SetSphere(SphereRadius);

							bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
								VRCharacterCapsule->GetComponentQuat(),
								ECollisionChannel::ECC_WorldDynamic,
								SphereTrace, SphereParams);

							if (bHitFloor)
							{
								float FloorAngle = ExtraMaths::GetAngleOfTwoVectors(FloorHit.ImpactNormal, FVector(0, 0, 1));

								if (FloorAngle <= MaxWalkableSlopeAngle && FloorAngle != 0)
								{
									PlaneProjectDir.Normalize();
									float MoveAngle = ExtraMaths::GetAngleOfTwoVectors(PlaneProjectDir, FVector(0, 0, 1));
									float AngleDifference = 90 - (MoveAngle - FloorAngle);
																						
									FVector RightDir = PlaneProjectDir;
									RightDir = RightDir.RotateAngleAxis(90, FVector(0, 0, 1));

									PlaneProjectDir = PlaneProjectDir.RotateAngleAxis(-AngleDifference, RightDir);

									Offset = PlaneProjectDir * ((WalkMovementSpeed * DeltaTime));
									bZRecenter = true;
								}
							}
						}

						VRCharacterCapsule->AddWorldOffset(Offset, true);

						if(bZRecenter)
							ZRecenter();
					}
					else
					{
						//move up a slope
						{
							/*FVector ImpactDir = Hit.ImpactNormal;
							FVector RotateAxis = Hit.Actor->GetActorRightVector();
							ImpactDir = ImpactDir.RotateAngleAxis(90, RotateAxis);
							float SlopeAngle = ExtraMaths::GetAngleOfTwoVectors(ImpactDir, FVector(0, 0, 1));

							FVector MoveDir = XYMovement;
							MoveDir.Normalize();
							float MoveAngle = ExtraMaths::GetAngleOfTwoVectors(MoveDir, FVector(0, 0, 1));

							FVector RightDir = MoveDir;
							RightDir = RightDir.RotateAngleAxis(90, FVector(0, 0, 1));

							float RotateAmount = MoveAngle - SlopeAngle;
							MoveDir = MoveDir.RotateAngleAxis(-RotateAmount, RightDir);*/
							FVector MyDir = Offset;
							MyDir.Normalize();
							FVector MoveDir = GetSlopeMovementDirection(MyDir, Hit.ImpactNormal, Hit.Actor->GetActorRightVector());
							FVector NewOffset = MoveDir * ((WalkMovementSpeed * DeltaTime) * (1 - Hit.Time));
							
							/*DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
									VRCharacterCapsule->GetComponentLocation() + (MoveDir * 1000), FColor::Green, false, 15.0f);

								DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
									VRCharacterCapsule->GetComponentLocation() + (RightDir * 1000), FColor::Red, false, 15.0f);

								DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
									VRCharacterCapsule->GetComponentLocation() + NewOffset, FColor::Red, false, 15.0f);

								DrawDebugLine(GetWorld(), Hit.GetActor()->GetActorLocation(),
									Hit.GetActor()->GetActorLocation() + (RotateAxis * 1000),
									FColor::Black, false, 15.0f);*/

							VRCharacterCapsule->AddWorldOffset(NewOffset, true);
							ZRecenter();
						}					
					}
				}
				
				if (bGrounded)
				{
					FHitResult FloorHit;
					FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
					float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
					FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 10));
					FCollisionShape SphereTrace;
					FCollisionQueryParams SphereParams;
					SphereParams.AddIgnoredActor(ComponentOwner);

					SphereTrace.SetSphere(SphereRadius);

					bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
						VRCharacterCapsule->GetComponentQuat(),
						ECollisionChannel::ECC_WorldDynamic,
						SphereTrace, SphereParams);

					if (bHitFloor)
					{
						FVector FloorDir = -FloorHit.Normal;
						FVector NewOffset = FloorDir * 100;
						/*DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
								VRCharacterCapsule->GetComponentLocation() + (MoveDir * 1000), FColor::Green, false, 15.0f);

							DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
								VRCharacterCapsule->GetComponentLocation() + (RightDir * 1000), FColor::Red, false, 15.0f);

							DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
								VRCharacterCapsule->GetComponentLocation() + NewOffset, FColor::Red, false, 15.0f);

							DrawDebugLine(GetWorld(), Hit.GetActor()->GetActorLocation(),
								Hit.GetActor()->GetActorLocation() + (RotateAxis * 1000),
								FColor::Black, false, 15.0f);*/

						VRCharacterCapsule->AddWorldOffset(NewOffset, true);
						ZRecenter();
					}

				}

				//XYRecenter();
			}
			else if(btest)
			{
				FVector MovetoLocation = ComponentOwner->GetActorLocation() + (XYMovement * DeltaTime);

				bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MovetoLocation, VRCharacterCapsule->GetComponentRotation());

				if (bFoundSpot)
				{
					ComponentOwner->SetActorLocation(MovetoLocation);
					XYRecenter();
				}
			}

		}
	}	

	FVector NewCapsuleLocation = VRCharacterCapsule->GetComponentLocation();
	NewCapsuleLocation.Z = 0;

	FVector OriginOffset = NewCapsuleLocation - OldCharacterCapsuleLoc;

	VRCharacterCameraOrigin->AddWorldOffset(OriginOffset);
}

void UVRCharacterComponent::SmoothRotation(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin)
		return;
	
	if (bSmoothTurning)
	{
		if (CurrentXRotationValue > RotationThumbstickDeadZone || CurrentXRotationValue < -RotationThumbstickDeadZone)
		{
			if (CurrentXRotationValue != 0)
			{
				//New Rotation
				FVector Distance = /*ComponentOwner->GetActorLocation()*/VRCharacterCameraOrigin->GetComponentLocation() - VRCharacterCamera->GetComponentLocation();
				FVector Rotation = Distance.RotateAngleAxis(((SmoothTurningSensitivity * 10) * CurrentXRotationValue) * DeltaTime, FVector(0, 0, 1));
				FVector FinalLocation = VRCharacterCamera->GetComponentLocation() + Rotation;

				//ComponentOwner->SetActorLocation(FinalLocation);
				//ComponentOwner->AddActorWorldRotation(FRotator(0, ((SmoothTurningSensitivity * 10) * CurrentXRotationValue) * DeltaTime, 0));
				VRCharacterCameraOrigin->SetWorldLocation(FinalLocation);
				VRCharacterCameraOrigin->AddRelativeRotation(FRotator(0, ((SmoothTurningSensitivity * 10) * CurrentXRotationValue) * DeltaTime, 0));
			}
		}
	}
	else
	{
		if (!bDoneSnapTurning)
		{
			if (CurrentXRotationValue > RotationThumbstickDeadZone || CurrentXRotationValue < -RotationThumbstickDeadZone)
			{
				if (CurrentXRotationValue != 0)
				{
					float SnapeTurningValue = SnapTurningAmount;

					if (CurrentXRotationValue > 0)
						SnapeTurningValue = SnapTurningAmount;
					else
						SnapeTurningValue = -SnapTurningAmount;

					//New Rotation
					FVector Distance = /*ComponentOwner->GetActorLocation()*/VRCharacterCameraOrigin->GetComponentLocation() - VRCharacterCamera->GetComponentLocation();
					FVector Rotation = Distance.RotateAngleAxis(SnapeTurningValue, FVector(0, 0, 1));
					FVector FinalLocation = VRCharacterCamera->GetComponentLocation() + Rotation;

					//ComponentOwner->SetActorLocation(FinalLocation);
					//ComponentOwner->AddActorWorldRotation(FRotator(0, SnapeTurningValue, 0));
					VRCharacterCameraOrigin->SetWorldLocation(FinalLocation);
					VRCharacterCameraOrigin->AddRelativeRotation(FRotator(0, SnapeTurningValue, 0));

					bDoneSnapTurning = true;
				}
			}
		}
		else if (CurrentXRotationValue < RotationThumbstickDeadZone && CurrentXRotationValue > -RotationThumbstickDeadZone)
		{
			bDoneSnapTurning = false;
		}
	}
}

void UVRCharacterComponent::ScaleCollisionWithPlayerNetworked(bool bLocallyControlled)
{
	if (bLocallyControlled)
	{
		float CameraZ = VRCharacterCamera->GetComponentLocation().Z;
		float ActorZ = VRCharacterCameraOrigin->GetComponentLocation().Z;
		float NewCapHeight = (CameraZ - ActorZ) / 2;
		ScaleCollisionWithPlayer(NewCapHeight);
		Server_SetNewHalfHeight(NewCapHeight);
	}
	else
	{
		if (ActorsCapHalfHeight != OldCapHalfHeight)
		{
			OldCapHalfHeight = ActorsCapHalfHeight;
			ScaleCollisionWithPlayer(ActorsCapHalfHeight);
		}
	}
}

void UVRCharacterComponent::ScaleCollisionWithPlayer(float NewHalfHeight)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin)
		return;

	/*Save old collision Hight for later*/
	float OldHeight = VRCharacterCapsule->GetScaledCapsuleHalfHeight();

	/*Work out new collision height*/
	//float CameraZ = VRCharacterCamera->GetComponentLocation().Z;
	//float ActorZ = VRCharacterCameraOrigin->GetComponentLocation().Z;
	float NewCapHeight = NewHalfHeight;//(CameraZ - ActorZ) / 2;
	VRCharacterCapsule->SetCapsuleHalfHeight(NewCapHeight);

	FVector CapLoc = VRCharacterCapsule->GetComponentLocation();
	CapLoc.Z = VRCharacterCameraOrigin->GetComponentLocation().Z + VRCharacterCapsule->GetScaledCapsuleHalfHeight();
	VRCharacterCapsule->SetWorldLocation(CapLoc);

	if (!ServerSideCap)
		return;

	ServerSideCap->SetCapsuleHalfHeight(NewHalfHeight);
	FVector ServerCapLoc = ServerSideCap->GetComponentLocation();
	ServerCapLoc.Z = VRCharacterCameraOrigin->GetComponentLocation().Z + ServerSideCap->GetScaledCapsuleHalfHeight();
	ServerSideCap->SetWorldLocation(ServerCapLoc);
}

void UVRCharacterComponent::MoveCollisionToHMD(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FVector ActorLocation = ComponentOwner->GetActorLocation();
	FVector CapsuleLocation = VRCharacterCapsule->GetComponentLocation();

	FVector HMDLocation = VRCharacterCamera->GetComponentLocation();
	HMDLocation.Z = CapsuleLocation.Z;

	FVector MoveToLoc = HMDLocation;
	MoveToLoc.Z = ComponentOwner->GetActorLocation().Z;

	FVector Dir = HMDLocation - CapsuleLocation;
	Dir.Z = 0;
	float Distance = Dir.Size();
	Dir.Normalize();
	MovePlayerCapsule(Dir, Distance, VRCharacterCapsule, false, true);

	if (LastCapLocation != VRCharacterCapsule->GetComponentLocation())
	{
		//MovePlayerCapsuleWithServerSynced(Dir, Distance, VRCharacterCapsule, false, true, DeltaTime);
		LastCapLocation = VRCharacterCapsule->GetComponentLocation();
	}
	
	if (!XYMovement.IsZero())
	{
		if (bUseSweepMovement)
		{
			//FVector Offset = HMDLocation - CapsuleLocation;
			//Offset.Z = 0;
			//VRCharacterCapsule->AddWorldOffset(Offset, true);		
			/*FVector Dir = HMDLocation - CapsuleLocation;
			float Distance = Dir.Size();
			Dir.Normalize();
			MovePlayerCapsule(Dir, Distance);*/
			//XYRecenter();

			//MovePlayerCapsuleWithServerSynced(Dir, Distance, VRCharacterCapsule, false, true, DeltaTime);
		}
		else
		{
			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MoveToLoc, VRCharacterCapsule->GetComponentRotation());
			if (bFoundSpot)
			{
				FVector OldOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();
				ComponentOwner->SetActorLocation(MoveToLoc);
				FVector NewOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();
				NewOriginLoc -= NewOriginLoc - OldOriginLoc;
				VRCharacterCameraOrigin->SetWorldLocation(NewOriginLoc);
			}
		}
	}
	else
	{
		/*FVector NewActorLoc = ComponentOwner->GetActorLocation();

		FVector Difference = NewActorLoc - ActorLocation;
		Difference.Z = 0;

		FVector NewOriginLoc = VRCharacterCameraOrigin->GetComponentLocation() - Difference;
		VRCharacterCameraOrigin->SetWorldLocation(NewOriginLoc);*/

		FVector OldOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();

		//sweep collision from actor location to HMD location
		if (bUseSweepMovement)
		{
			FHitResult Hit;
			FVector Offset = HMDLocation - CapsuleLocation;
			Offset.Z = 0;
			//VRCharacterCapsule->AddWorldOffset(Offset, true, &Hit);

			if (Hit.bBlockingHit)
			{
				FVector Normal2D = Hit.Normal;
				Normal2D.Z = 0;
				Normal2D.Normalize();

				Offset = FVector::VectorPlaneProject(Offset, Normal2D);
				Offset = Offset * (1 - Hit.Time);
				//VRCharacterCapsule->AddWorldOffset(Offset, true);
			}
		}
		else
		{
			FHitResult ObjectHit;
			FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
			float CapHalfHeight = VRCharacterCapsule->GetScaledCapsuleHalfHeight() - 0.5f;
			float CapRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 0.5f;
			FVector CamLoc = VRCharacterCamera->GetComponentLocation();
			CamLoc.Z = StartLocation.Z;
			FVector EndLocation = CamLoc;

			FCollisionShape CapsuleTrace;
			CapsuleTrace.SetCapsule(FVector(1, 1, 1));
			CapsuleTrace.Capsule.HalfHeight = CapHalfHeight;
			CapsuleTrace.Capsule.Radius = CapRadius;
			FCollisionQueryParams SphereParams;
			SphereParams.AddIgnoredActor(ComponentOwner);

			bool bHitSomething = GetWorld()->SweepSingleByChannel(ObjectHit, StartLocation, EndLocation,
				VRCharacterCapsule->GetComponentQuat(),
				ECollisionChannel::ECC_Visibility,
				CapsuleTrace, SphereParams);
		
			if (bHitSomething)
			{

				bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, ObjectHit.Location, VRCharacterCapsule->GetComponentRotation());
				
				if(bFoundSpot)
					ComponentOwner->SetActorLocation(ObjectHit.Location);
			}
			else
			{
				bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MoveToLoc, VRCharacterCapsule->GetComponentRotation());
				
				if(bFoundSpot)
					ComponentOwner->SetActorLocation(MoveToLoc);
			}
		}

		FVector NewOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();
		NewOriginLoc -= NewOriginLoc - OldOriginLoc;
		VRCharacterCameraOrigin->SetWorldLocation(NewOriginLoc);

		CheckHMDDistanceFromCollision();
	}
}

void UVRCharacterComponent::CheckHMDDistanceFromCollision()
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin)
		return;

	FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
	FVector EndLocation = VRCharacterCamera->GetComponentLocation();
	StartLocation.Z = EndLocation.Z;

	float Distance = FVector::Distance(StartLocation, EndLocation);
	if (Distance > MaxCameraDistanceFromCollision)
	{
		if (SnapCameraBackWhenInWrongLocation)
		{
			XYRecenter();
			//ZRecenter();
		}
		else
		{
			FVector Dir = StartLocation - EndLocation;
			Dir.Normalize();
			
			float Offset = Distance - MaxCameraDistanceFromCollision;
			VRCharacterCameraOrigin->AddWorldOffset(Dir * Offset);

		}
	}
}

void UVRCharacterComponent::CheckToSeeIfCameraIsInsideObject()
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FHitResult Hit;
	FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
	StartLocation.Z += VRCharacterCapsule->GetScaledCapsuleHalfHeight();
	float SphereRadius = CameraSphereCollisionRadius;
	FVector EndLocation =  VRCharacterCamera->GetComponentLocation();
	FCollisionShape SphereTrace;
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActor(ComponentOwner);

	SphereTrace.SetSphere(SphereRadius);

	bool bHitSomething = GetWorld()->SweepSingleByChannel(Hit, StartLocation, EndLocation,
		VRCharacterCapsule->GetComponentQuat(),
		ECollisionChannel::ECC_WorldDynamic,
		SphereTrace, SphereParams);

	if (bHitSomething)
	{
		if (SnapCameraBackWhenInWrongLocation)
		{
			XYRecenter();
			//ZRecenter();
		}
		else
		{
			FVector Offset = Hit.Location - VRCharacterCamera->GetComponentLocation();
			Offset.Z = 0;
			VRCharacterCameraOrigin->AddWorldOffset(Offset);
		}

	}
}

void UVRCharacterComponent::MovePlayerCapsuleWithServerSynced(FVector Dir, float OffsetAmount, 
	UCapsuleComponent* CapToMove, bool bXYRecenter, bool bZRecenter, float DeltaTime, bool bIsGravity)
{
	FPlayerMove Move;
	Move.StartLocation = VRCharacterCapsule->GetComponentLocation();

	MovePlayerCapsule(Dir, OffsetAmount, CapToMove, bXYRecenter, bZRecenter);

	Move.DeltaTime = DeltaTime;
	Move.Dir = Dir;

	Move.EndLocation = VRCharacterCapsule->GetComponentLocation();

	if (GetOwnerRole() >= ROLE_Authority)
	{
		FClientSide_Prediction NewMove;
		NewMove.LastServerWorldDelta = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();
		NewMove.LastServerLocation = Move.EndLocation;
		NewMove.Dir = Move.Dir;
		NewMove.bStillMoving = true;

		GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Blue, "Host sending move", true);
		NetMulticast_SendMove(NewMove.Dir, NewMove.LastServerLocation, NewMove.LastServerWorldDelta, 
			NewMove.bStillMoving, bIsGravity);

		//if (ServerSideCap)
		//	ServerSideCap->SetWorldLocation(Move.EndLocation);
	}
	else
	{
		Server_SendMove(Move.Dir, Move.DeltaTime, Move.EndLocation, bIsGravity);
	}

}

void UVRCharacterComponent::MovePlayerCapsule(FVector Dir, float OffsetAmount, 
	UCapsuleComponent* CapToMove, bool bXYRecenter, bool bZRecenter)
{
	if (!CapToMove || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FHitResult FinalMove;
	FVector StartLocation = CapToMove->GetComponentLocation();
	FVector EndLocation = StartLocation + (Dir * OffsetAmount);

	if (bUseSweepMovement)
	{
		//FHitResult Hit;
		FVector Offset = Dir * OffsetAmount;
		CapToMove->AddWorldOffset(Offset, true, &FinalMove);

		if (FinalMove.bBlockingHit)
		{
			StartLocation = FinalMove.Location;
			
			float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), FinalMove.ImpactNormal);
	
			//GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "Slope Angle: " + FString::SanitizeFloat(Angle), true);
	
			if (Angle > MaxWalkableSlopeAngle)
			{
				FVector Normal2D = FinalMove.Normal;
				Normal2D.Z = 0;
				Normal2D.Normalize();
	
				FVector PlaneProject = FVector::VectorPlaneProject(Offset, Normal2D);
				Offset = PlaneProject * (1 - FinalMove.Time);
				bool bShouldZRecenter = false;
	
				/* Check to see if their is a slope below us */
				{
					FHitResult FloorHit;
					FVector SphereStartLocation = StartLocation;
					float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
					FVector SphereEndLocation = SphereStartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 2.5f));
					SphereCast(FloorHit, SphereStartLocation, SphereEndLocation, SphereRadius);
	
					if (FloorHit.bBlockingHit)
					{
						float FloorAngle = ExtraMaths::GetAngleOfTwoVectors(FloorHit.ImpactNormal, FVector(0, 0, 1));
	
						if (FloorAngle <= MaxWalkableSlopeAngle && FloorAngle != 0)
						{
							FVector PPDir = PlaneProject;
							PPDir.Normalize();
							FVector RotateAxis = PPDir.RotateAngleAxis(90, FVector(0, 0, 1));
							FVector NewDir = GetSlopeMovementDirection(PPDir, FloorHit.ImpactNormal, RotateAxis);
							//Offset = (NewDir  * PlaneProject) * (1 - Hit.Time);
							float RotateAmount = 90 -
								(ExtraMaths::GetAngleOfTwoVectors(PPDir, FVector(0, 0, 1)) - FloorAngle);

							FVector RotatedF = PlaneProject.RotateAngleAxis(-RotateAmount, RotateAxis);
							Offset = RotatedF * (1 - FinalMove.Time);
							bShouldZRecenter = true;

							/*DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
								VRCharacterCapsule->GetComponentLocation() + Offset, FColor::Blue,
								false, 50.0f);*/
						}
					}
				}

				CapToMove->AddWorldOffset(Offset, true);

				if (bZRecenter && bShouldZRecenter)
					ZRecenter();
			}
			else
			{
				//move up a slope
				if (FinalMove.Actor != NULL)
				{
					FVector NewDir = GetSlopeMovementDirection(Dir, FinalMove.ImpactNormal, FinalMove.Actor->GetActorRightVector());
					FVector NewOffset = (NewDir * OffsetAmount) * (1 - FinalMove.Time);

					/*DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
						VRCharacterCapsule->GetComponentLocation() + NewOffset, FColor::Red,
						false, 0.5f);*/

					CapToMove->AddWorldOffset(NewOffset, true);

					if (bZRecenter)
						ZRecenter();
				}
			}
		}
	
		if (bGrounded)
		{
			FHitResult FloorHit;
			FVector SphereStartLocation = StartLocation;
			float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
			FVector SphereEndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 10));
			SphereCast(FloorHit, SphereStartLocation, SphereEndLocation, SphereRadius);
	
			if (FloorHit.bBlockingHit)
			{
				FVector FloorDir = -FloorHit.Normal;
				FVector NewOffset = FloorDir * 100;
				CapToMove->AddWorldOffset(NewOffset, true);
				
				if (bZRecenter)
					ZRecenter();
			}
	
		}
	
		if(bXYRecenter)
			XYRecenter();
	}
}

FVector UVRCharacterComponent::GetSlopeMovementDirection(FVector CurrentDir, FVector SlopeDir, FVector RotateAxis)
{
	FVector ImpactDir = SlopeDir;
	ImpactDir = ImpactDir.RotateAngleAxis(90, RotateAxis);
	float SlopeAngle = ExtraMaths::GetAngleOfTwoVectors(ImpactDir, FVector(0, 0, 1));

	FVector MoveDir = CurrentDir;
	float MoveAngle = ExtraMaths::GetAngleOfTwoVectors(MoveDir, FVector(0, 0, 1));

	FVector RightDir = MoveDir;
	RightDir = RightDir.RotateAngleAxis(90, FVector(0, 0, 1));

	float RotateAmount = MoveAngle - SlopeAngle;
	MoveDir = MoveDir.RotateAngleAxis(-RotateAmount, RightDir);

	/*DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
			VRCharacterCapsule->GetComponentLocation() + (MoveDir * 1000), FColor::Green, false, 15.0f);

		DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
			VRCharacterCapsule->GetComponentLocation() + (RightDir * 1000), FColor::Red, false, 15.0f);

		DrawDebugLine(GetWorld(), VRCharacterCapsule->GetComponentLocation(),
			VRCharacterCapsule->GetComponentLocation() + NewOffset, FColor::Red, false, 15.0f);

		DrawDebugLine(GetWorld(), Hit.GetActor()->GetActorLocation(),
			Hit.GetActor()->GetActorLocation() + (RotateAxis * 1000),
			FColor::Black, false, 15.0f);*/

	return MoveDir;
}

void UVRCharacterComponent::ApplyGravity(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	//Check for the floor and apply gravity if needed
	FHitResult FloorHit;
	FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
	float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
	FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 2.5f));
	SphereCast(FloorHit, StartLocation, EndLocation, SphereRadius);

	if (bUseSweepMovement)
	{
		if (!FloorHit.bBlockingHit)
		{
			bGrounded = false;
			FVector Offset = FVector(0, 0, 1) * (GravitySpeed * DeltaTime);
			//VRCharacterCapsule->AddWorldOffset(Offset, true);	
			
			MovePlayerCapsuleWithServerSynced(FVector(0, 0, 1), GravitySpeed * DeltaTime, VRCharacterCapsule,
				false, true, DeltaTime, true);

			ZRecenter();
		}
		else if (!bGrounded)
		{
			bGrounded = true;
			FVector Offset = FVector(0, 0, 1) * (EndLocation.Z - StartLocation.Z);
			//VRCharacterCapsule->AddWorldOffset(Offset, true);

			MovePlayerCapsuleWithServerSynced(FVector(0, 0, 1), Offset.Size(), VRCharacterCapsule,
				false, true, DeltaTime);

			ZRecenter();
		}
	}
	else
	{
		if (!FloorHit.bBlockingHit)
		{
			FVector MovetoLocation = ComponentOwner->GetActorLocation() +
				(FVector(0, 0, 1) * (GravitySpeed * DeltaTime));

			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MovetoLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				if (bUseSweepMovement)
				{
					FVector Offset = MovetoLocation - ComponentOwner->GetActorLocation();
					VRCharacterCapsule->AddWorldOffset(Offset, true);
				}
				else
					ComponentOwner->SetActorLocation(MovetoLocation);
				//ZRecenter();
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
			}
		}
		else
		{
			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, FloorHit.Location, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				if (bUseSweepMovement)
				{
					FVector Offset = FloorHit.Location - ComponentOwner->GetActorLocation();
					VRCharacterCapsule->AddWorldOffset(Offset, true);
				}
				else
					ComponentOwner->SetActorLocation(FloorHit.Location);
				//ZRecenter();
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
			}
		}
	}
}

void UVRCharacterComponent::SphereCast(FHitResult& Result, FVector StartLoc, FVector EndLoc, float SphereRadius)
{
	FCollisionShape SphereTrace;
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActor(ComponentOwner);

	SphereTrace.SetSphere(SphereRadius);

	GetWorld()->SweepSingleByChannel(Result, StartLoc, EndLoc, VRCharacterCapsule->GetComponentQuat(),
		ECollisionChannel::ECC_GameTraceChannel1, SphereTrace, SphereParams);
}

void UVRCharacterComponent::Server_SetHMDLocation_Implementation(FVector SetHMDLocation)
{
	SyncedHMDLocation = SetHMDLocation;
}

void UVRCharacterComponent::Server_SetNewHalfHeight_Implementation(float NewHalfHeight)
{
	ActorsCapHalfHeight = NewHalfHeight;
}

void UVRCharacterComponent::Client_SetLocation_Implementation(FVector Location)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FVector OldCharacterCapsuleLoc = VRCharacterCapsule->GetComponentLocation();

	VRCharacterCapsule->SetWorldLocation(Location);

	FVector NewCapsuleLocation = VRCharacterCapsule->GetComponentLocation();

	FVector OriginOffset = NewCapsuleLocation - OldCharacterCapsuleLoc;

	VRCharacterCameraOrigin->AddWorldOffset(OriginOffset);
}

void UVRCharacterComponent::NetMulticast_SendMove_Implementation(FVector Dir, FVector LastServerLocation,
	float LastServerWorldDelta, bool bStillMoving, bool bIsGravity)
{
	CurrentProxyMove.Dir = Dir;
	CurrentProxyMove.LastServerLocation = LastServerLocation;
	CurrentProxyMove.LastServerWorldDelta = LastServerWorldDelta;
	CurrentProxyMove.bStillMoving = bStillMoving;
	CurrentProxyMove.bIsGravity = bIsGravity;

	/*if (bLast != LastKnownMove.bStillMoving)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
			LastKnownMove.ToString(), true);
		bLast = LastKnownMove.bStillMoving;
	}*/

	if (UpdateMultiTime <= 0)
	{
		/*GEngine->AddOnScreenDebugMessage(49, 5.0f, FColor::Purple,
			CurrentProxyMove.ToString(), true);*/
		UpdateMultiTime = 1;
	}

	WorkOutSimulatedProxyError();
}

void UVRCharacterComponent::Server_SendMove_Implementation(FVector Dir, float DeltaTime, FVector EndLocation, bool bIsGravity)
{
	FPlayerMove Move;
	Move.Dir = Dir;
	Move.EndLocation = EndLocation;
	Move.DeltaTime = DeltaTime;
	Move.bIsGravity = bIsGravity;
	//ClientsMoves.Add(Move);

	FClientSide_Prediction NewMove;
	NewMove.LastServerWorldDelta = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();
	NewMove.LastServerLocation = SyncedServerLocation;
	NewMove.Dir = Move.Dir;

	if (Move.Dir == FVector::ZeroVector)
		NewMove.bStillMoving = false;
	else
		NewMove.bStillMoving = true;

	NetMulticast_SendMove(NewMove.Dir, EndLocation, NewMove.LastServerWorldDelta,
		NewMove.bStillMoving, NewMove.bIsGravity);
}

void UVRCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRCharacterComponent, SyncedServerLocation);
	DOREPLIFETIME(UVRCharacterComponent, ActorsCapHalfHeight);
	DOREPLIFETIME(UVRCharacterComponent, SyncedHMDLocation);
}