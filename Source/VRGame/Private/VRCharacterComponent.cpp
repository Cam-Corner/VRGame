// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacterComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UVRCharacterComponent::UVRCharacterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UVRCharacterComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentMovementMode = EMovementModes::EMM_Walk;


	// ...
	
}

// Called every frame
void UVRCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//HandleMovement(DeltaTime);
	ScaleCollisionWithPlayer();
	HandleMovement(DeltaTime);
	MoveCollisionToHMD();
	SmoothRotation(DeltaTime);

	// ...
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
	MoveOffset.Z = VRCharacterCameraOrigin->GetComponentLocation().Z -
		(VRCharacterCapsule->GetComponentLocation().Z - VRCharacterCapsule->GetScaledCapsuleHalfHeight());

	VRCharacterCameraOrigin->AddWorldOffset(MoveOffset);
}

void UVRCharacterComponent::SetXYMovementDirection(FVector2D Dir)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	GEngine->AddOnScreenDebugMessage(20, 0.1f, FColor::Blue, Dir.ToString(), true);

	float ThumbstickDeadzone = 0.2f;

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

void UVRCharacterComponent::HandleMovement(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	/*Set the forward rotation to the Yaw of the camera*/
	FRotator NewRotation = VRCharacterCapsule->GetComponentRotation();
	NewRotation.Yaw = VRCharacterCamera->GetComponentRotation().Yaw;
	VRCharacterCapsule->SetWorldRotation(NewRotation);
	
	//Apply the desired movement
	{
		if (!XYMovement.IsZero())
		{
			XYRecenter();
			FVector MovetoLocation = ComponentOwner->GetActorLocation() + (XYMovement * DeltaTime);

			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MovetoLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				ComponentOwner->SetActorLocation(MovetoLocation);
				XYRecenter();
			}
		}
	}

	//Check for the floor and apply gravity if needed
	{
		FHitResult FloorHit;
		FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
		float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
		FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 1.5f));
		FCollisionShape SphereTrace;
		FCollisionQueryParams SphereParams;
		SphereParams.AddIgnoredActor(ComponentOwner);

		SphereTrace.SetSphere(SphereRadius);

		bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
			VRCharacterCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Visibility,
			SphereTrace, SphereParams);

		if (!bHitFloor)
		{
			FVector MovetoLocation = ComponentOwner->GetActorLocation() +
				(FVector(0, 0, 1) * (GravitySpeed * DeltaTime));

			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MovetoLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				ComponentOwner->SetActorLocation(MovetoLocation);
				//ZRecenter();
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
			}
		}
		else
		{
			bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, EndLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				ComponentOwner->SetActorLocation(EndLocation);
				//ZRecenter();
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
			}
		}

	}
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

void UVRCharacterComponent::ScaleCollisionWithPlayer()
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin)
		return;

	/*Save old collision Hight for later*/
	float OldHeight = VRCharacterCapsule->GetScaledCapsuleHalfHeight();

	/*Work out new collision height*/
	float CameraZ = VRCharacterCamera->GetComponentLocation().Z;
	float ActorZ = VRCharacterCameraOrigin->GetComponentLocation().Z;
	float NewCapHeight = (CameraZ - ActorZ) / 2;
	VRCharacterCapsule->SetCapsuleHalfHeight(NewCapHeight);

	FVector CapLoc = VRCharacterCapsule->GetComponentLocation();
	CapLoc.Z = VRCharacterCameraOrigin->GetComponentLocation().Z + VRCharacterCapsule->GetScaledCapsuleHalfHeight();
	VRCharacterCapsule->SetWorldLocation(CapLoc);
}

void UVRCharacterComponent::MoveCollisionToHMD()
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !ComponentOwner)
		return;

	FVector ActorLocation = ComponentOwner->GetActorLocation();

	FVector HMDLocation = VRCharacterCamera->GetComponentLocation();
	HMDLocation.Z = ActorLocation.Z;

	FVector MoveToLoc = HMDLocation;
	MoveToLoc.Z = ComponentOwner->GetActorLocation().Z;

	if (!XYMovement.IsZero())
	{
		bool bFoundSpot = GetWorld()->FindTeleportSpot(ComponentOwner, MoveToLoc, VRCharacterCapsule->GetComponentRotation());
		if (bFoundSpot)
		{
			FVector OldOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();
			ComponentOwner->SetActorLocation(MoveToLoc);
			FVector NewOriginLoc = VRCharacterCameraOrigin->GetComponentLocation();
			NewOriginLoc -=  NewOriginLoc - OldOriginLoc;
			VRCharacterCameraOrigin->SetWorldLocation(NewOriginLoc);
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
	if (!VRCharacterCapsule || !VRCharacterCamera)
		return;

	FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
	FVector EndLocation = VRCharacterCamera->GetComponentLocation();
	StartLocation.Z = EndLocation.Z;

	if (FVector::Distance(StartLocation, EndLocation) > MaxCameraDistanceFromCollision)
		XYRecenter();
}

