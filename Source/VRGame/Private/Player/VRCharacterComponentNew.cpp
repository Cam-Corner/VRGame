// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRCharacterComponentNew.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Saving/ControlSettingsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/ExtraMaths.h"
#include "DrawDebugHelpers.h"
#include "Networking/NetworkingHelpers.h"
#include "Player/VRCharacter.h"

DEFINE_LOG_CATEGORY(LogAVRCharacterComponentNew);

// Sets default values for this component's properties
UVRCharacterComponentNew::UVRCharacterComponentNew()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVRCharacterComponentNew::BeginPlay()
{
	Super::BeginPlay();

	CurrentMovementType = DefaultMovementType;
	CurrentGroundedType = DefaultGroundedType;

	// ...
	
}

// Called every frame
void UVRCharacterComponentNew::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ENetRole Role = GetOwnerRole();

	if (OwningPawn)
	{
		if (OwningPawn->IsLocallyControlled())
		{
			OwningPlayerHandler(DeltaTime);
		}
		else if (Role == ROLE_SimulatedProxy || Role >= ROLE_Authority)
		{
			SimProxyHandler(DeltaTime);

			if (!Camera)
				GEngine->AddOnScreenDebugMessage(2, .01f, FColor::Red, "Camera NULL", true);
			else
				GEngine->AddOnScreenDebugMessage(2, .01f, FColor::Red, "Camera", true);

			if (!CameraOrigin)
				GEngine->AddOnScreenDebugMessage(3, .01f, FColor::Red, "CameraOrigin NULL", true);
			else
				GEngine->AddOnScreenDebugMessage(3, .01f, FColor::Red, "CameraOrigin", true);

			if (!Capsule)
				GEngine->AddOnScreenDebugMessage(4, .01f, FColor::Red, "Capsule NULL", true);
			else
				GEngine->AddOnScreenDebugMessage(4, .01f, FColor::Red, "Capsule", true);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, .01f, FColor::Red, "OwningPawn NULL", true);
	}
	
	// ...
}

void UVRCharacterComponentNew::SetCameraComponent(UCameraComponent* InCamera)
{
	Camera = InCamera;
}

void UVRCharacterComponentNew::SetCameraOriginComponent(USceneComponent* InCameraOrigin)
{
	CameraOrigin = InCameraOrigin;
}

void UVRCharacterComponentNew::SetCapsuleComponent(UCapsuleComponent* InCapsule)
{
	Capsule = InCapsule;
}

void UVRCharacterComponentNew::SetServerDebugMeshComponent(UStaticMeshComponent* InMesh)
{
	ServerDebugMesh = InMesh;
}

void UVRCharacterComponentNew::SetOwningPawn(APawn* InPawn)
{
	OwningPawn = InPawn;
}

void UVRCharacterComponentNew::MoveCapsule(UCapsuleComponent* CapToMove, const FVector Dir, const float MoveAmount, const bool bXYRecenter, bool bZRecenter)
{
	if (!CapToMove || !Camera || !CameraOrigin || !OwningPawn)
		return;

	FVector FixedDir = Dir;

	if (CurrentMovementType != EMovementType::EMM_Falling)
	{
		FHitResult FloorHit;
		FVector SLoc = CapToMove->GetComponentLocation();
		FVector ELoc = SLoc - (FVector(0, 0, 250));
		SphereCast(FloorHit, SLoc, ELoc, CapToMove->GetScaledCapsuleRadius() - 1, bShowDebugMovement, FColor::Red);

		float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), FloorHit.ImpactNormal);

		if (FloorHit.bBlockingHit && Angle != 0)
		{
			if (FloorHit.Normal.Z != 1)
			{
				FVector Normal = FloorHit.Normal;
				Normal.Z = 0;
				Normal.Normalize();

				FixedDir = FVector::VectorPlaneProject(FixedDir, FloorHit.Normal);
			}
		}
	}

	FHitResult MovementHit;
	FVector Offset = FixedDir * MoveAmount;
	CapToMove->AddWorldOffset(Offset, true, &MovementHit);

	if (MovementHit.bBlockingHit)
	{
		if (IsWalkableSurface(MovementHit))
		{
			FVector PlaneProject = FVector::VectorPlaneProject(Offset, MovementHit.Normal);
			Offset = PlaneProject * (1 - MovementHit.Time);
			CapToMove->AddWorldOffset(Offset, true, &MovementHit);

			if (MovementHit.bBlockingHit)
			{
				FVector Normal = MovementHit.Normal;
				Normal.Z = 0;
				Normal.Normalize();

				FVector PlaneProject2 = FVector::VectorPlaneProject(Offset, Normal);
				Offset = PlaneProject2 * (1 - MovementHit.Time);
				CapToMove->AddWorldOffset(Offset, true);
			}
		}
		else
		{
			FHitResult Result;
			FVector SLoc = MovementHit.ImpactPoint + (Dir * MoveAmount);
			SLoc.Z = CapToMove->GetComponentLocation().Z;
			SLoc.Z += MaxStepUpHeight + SMALL_NUMBER;
			FVector ELoc = SLoc;
			ELoc.Z = CapToMove->GetComponentLocation().Z - CapToMove->GetScaledCapsuleHalfHeight();
			ShapeCast(Result, SLoc, ELoc, CapToMove->GetCollisionShape(), false, FColor::Red, .1f);

			if (IsWalkableSurface(Result) && Result.ImpactPoint.Z > CapToMove->GetComponentLocation().Z - CapToMove->GetScaledCapsuleHalfHeight())
			{
				float ZOffset = Result.ImpactPoint.Z -
					(CapToMove->GetComponentLocation().Z - CapToMove->GetScaledCapsuleHalfHeight());

				CapToMove->AddWorldOffset(FVector(0, 0, ZOffset), true);
				CapToMove->AddWorldOffset(Dir * MoveAmount, true);
			}
			
			FVector Normal = MovementHit.Normal;
			Normal.Z = 0;
			Normal.Normalize();

			FVector PlaneProject = FVector::VectorPlaneProject(Offset, Normal);
			Offset = PlaneProject * (1 - MovementHit.Time);
			CapToMove->AddWorldOffset(Offset, true, &MovementHit);

			if (MovementHit.bBlockingHit)
			{
				FVector PlaneProject2 = FVector::VectorPlaneProject(Offset, MovementHit.Normal);
				Offset = PlaneProject2 * (1 - MovementHit.Time);
				CapToMove->AddWorldOffset(Offset, true);
			}
		}
	}

	if (bXYRecenter)
		XYRecenter();

	if (bZRecenter)
		ZRecenter();
}

bool UVRCharacterComponentNew::IsWalkableSurface(const FHitResult& Hit)
{
	if (!Hit.IsValidBlockingHit() || Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
		return false;

	float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), Hit.ImpactNormal);
	if (Angle > MaxWalkableFloorAngle)
	{
		return false;
	}

	return true;
}

void UVRCharacterComponentNew::UpdateProxyMoveData(FVector CurrentLocation, FVector Dir, float CapHalfHeight)
{
	LastProxyUpdate.LastLocation = CurrentLocation;
	LastProxyUpdate.Direction = Dir;
	LastProxyUpdate.bGoToLastLocation = true;

	//ScaleCapsuleHeight(CapHalfHeight);
	{
		if (!Capsule || !Camera || !CameraOrigin)
			return;

		/*Save old collision Hight for later*/
		float OldHeight = Capsule->GetScaledCapsuleHalfHeight();
		Capsule->SetCapsuleHalfHeight(CapHalfHeight);
		float OffsetNeeded = CapHalfHeight - OldHeight;
		Capsule->AddWorldOffset(FVector(0, 0, OffsetNeeded));
	}

	FVector CapLoc = Capsule->GetComponentLocation();
	float Error = FVector::Distance(CapLoc, CurrentLocation);

	if (Error > MaxLocationError)
	{
		Capsule->SetWorldLocation(CurrentLocation);
	}
	//GEngine->AddOnScreenDebugMessage(5, .01f, FColor::Red, "UpdateProxyMoveData()", true);
}

void UVRCharacterComponentNew::Server_SendMove_Implementation(FVector CurrentLocation, FVector Dir, float CapHalfHeight)
{
	NetMulticast_SendMove(CurrentLocation, Dir, CapHalfHeight);
}

void UVRCharacterComponentNew::NetMulticast_SendMove_Implementation(FVector CurrentLocation, FVector Dir, float CapHalfHeight)
{
	UpdateProxyMoveData(CurrentLocation, Dir, CapHalfHeight);
}

void UVRCharacterComponentNew::SphereCast(FHitResult& Result, FVector StartLoc, FVector EndLoc, 
	float SphereRadius, bool bShowDebug, FColor Colour, float DebugTimer)
{
	FCollisionShape SphereTrace;
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActor(OwningPawn);

	SphereTrace.SetSphere(SphereRadius);

	GetWorld()->SweepSingleByChannel(Result, StartLoc, EndLoc, Capsule->GetComponentQuat(),
		ECollisionChannel::ECC_GameTraceChannel1, SphereTrace, SphereParams);

	if (bShowDebug)
	{
		/*if(Result.bBlockingHit)
			DrawDebugSphere(GetWorld(), Result.Location, SphereRadius, 16, Colour, false, DebugTimer);
		else
			DrawDebugSphere(GetWorld(), Result.TraceEnd, SphereRadius, 16, Colour, false, DebugTimer);*/

		if(Result.Actor != NULL)
			GEngine->AddOnScreenDebugMessage(1, DebugTimer, FColor::Yellow, Result.Actor->GetName(), true);
	}

}

void UVRCharacterComponentNew::ShapeCast(FHitResult& Result, FVector StartLoc, FVector EndLoc, FCollisionShape Shape,
	bool bShowDebug, FColor Colour, float DebugTimer)
{
	FCollisionQueryParams CapsuleParams;
	CapsuleParams.AddIgnoredActor(OwningPawn);
	CapsuleParams.AddIgnoredComponent(Capsule);
	CapsuleParams.bFindInitialOverlaps = false;
	CapsuleParams.bTraceComplex = false;

	GetWorld()->SweepSingleByChannel(Result, StartLoc, EndLoc, Capsule->GetComponentQuat(),
		ECollisionChannel::ECC_GameTraceChannel1, Shape, CapsuleParams);

	if (bShowDebug)
	{
		if (Result.bBlockingHit)
		{
			DrawDebugCapsule(GetWorld(), StartLoc, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), FQuat(), FColor::Blue, false, DebugTimer);
			DrawDebugSphere(GetWorld(), Result.ImpactPoint, Shape.GetCapsuleRadius(), 16, Colour, false, DebugTimer);
		}
		else
		{
			DrawDebugCapsule(GetWorld(), Result.TraceStart, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), FQuat(), FColor::Green, false, DebugTimer);
			DrawDebugCapsule(GetWorld(), Result.TraceEnd, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), FQuat(), Colour, false, DebugTimer);
			
		}
		if (Result.Actor != NULL)
			GEngine->AddOnScreenDebugMessage(2, DebugTimer, FColor::Yellow, Result.Actor->GetName(), true);
	}

}

void UVRCharacterComponentNew::HandleGroundedMode(float DeltaTime)
{
	if (!Capsule || !Camera || !OwningPawn)
		return;

	ENetRole Role = GetOwnerRole();

	/** check to see if we are still grounded */
	FHitResult Hit;
	SphereCast(Hit, Capsule->GetComponentLocation(),
		Capsule->GetComponentLocation() - FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight() - 1),
		Capsule->GetScaledCapsuleRadius(), bShowDebugMovement);

	if (!Hit.bBlockingHit)
		CurrentMovementType = EMovementType::EMM_Falling;

	if (OwningPawn->IsLocallyControlled())
	{
		/*Set the forward rotation to the Yaw of the camera*/
		FRotator NewRotation = Capsule->GetComponentRotation();
		NewRotation.Yaw = Camera->GetComponentRotation().Yaw;
		Capsule->SetWorldRotation(NewRotation);

		/* get the new input movement values */
		FVector2D InputDir = ConsumeMovementVector();
		
		if ((InputDir.X < MovementDeadZone && InputDir.Y < MovementDeadZone)
			&& (InputDir.X > -MovementDeadZone && InputDir.Y > -MovementDeadZone)
			|| InputDir == FVector2D::ZeroVector)
		{
			if (LastInputDir != InputDir)
			{
				Server_SendMove(Capsule->GetComponentLocation(), FVector::ZeroVector, Capsule->GetScaledCapsuleHalfHeight());
				SendMoveTimer = 0.0f;
				LastInputDir = InputDir;
			}

			return;
		}


		LastInputDir = InputDir;
		FVector MovementDir = FVector::ZeroVector;

		if (InputDir.Y != 0)
			MovementDir += Capsule->GetForwardVector() * InputDir.Y;

		if (InputDir.X != 0)
			MovementDir += Capsule->GetRightVector() * InputDir.X;

		if (MovementDir.Size() > 1)
			MovementDir.Normalize();

		if (InputDir != FVector2D::ZeroVector)
			MoveCapsule(Capsule, MovementDir, DefaultWalkSpeed * DeltaTime, true, true);	

		if (SendMoveTimer <= 0)
		{
			Server_SendMove(Capsule->GetComponentLocation(), MovementDir, Capsule->GetScaledCapsuleHalfHeight());
			SendMoveTimer = 0.1f;
		}
		else
			SendMoveTimer -= DeltaTime;
	}
	else if (Role == ROLE_SimulatedProxy || Role >= ROLE_Authority)
	{
		if (LastProxyUpdate.bGoToLastLocation)
		{
			FVector DirToCap = LastProxyUpdate.LastLocation - Capsule->GetComponentLocation();
			DirToCap.Z = 0;
			DirToCap.Normalize();
			float CurrentOffset = DefaultWalkSpeed * DeltaTime;
			float Distance = FVector::Distance(Capsule->GetComponentLocation(), LastProxyUpdate.LastLocation);
			
			if(CurrentOffset < Distance)
				MoveCapsule(Capsule, DirToCap, CurrentOffset, false, false);
			else
			{
				float LeftOver = CurrentOffset - Distance;
				MoveCapsule(Capsule, DirToCap, Distance, false, false);
				MoveCapsule(Capsule, LastProxyUpdate.Direction, LeftOver, false, false);
				LastProxyUpdate.bGoToLastLocation = false;
			}
		}
		else
		{
			if (LastProxyUpdate.Direction != FVector::ZeroVector)
				MoveCapsule(Capsule, LastProxyUpdate.Direction, DefaultWalkSpeed * DeltaTime, false, false);
		}	
	}

}

void UVRCharacterComponentNew::HandleFallingMode(float DeltaTime)
{
	if (!Capsule)
		return;

	MoveCapsule(Capsule, FVector(0, 0, 1), GravitySpeed * DeltaTime, true, true);

	FHitResult Hit;
	SphereCast(Hit, Capsule->GetComponentLocation(),
		Capsule->GetComponentLocation() - FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight()),
		Capsule->GetScaledCapsuleRadius() - 1, bShowDebugMovement);

	if (Hit.bBlockingHit)
	{
		CurrentMovementType = EMovementType::EMM_Grounded;
		MoveCapsule(Capsule, FVector(0, 0, 1), GravitySpeed, true, true);
	}
}

void UVRCharacterComponentNew::MoveCollisionToHmd()
{
	if (!Capsule || !Camera || !CameraOrigin || !OwningPawn)
		return;

	FVector Difference = Camera->GetComponentLocation() - Capsule->GetComponentLocation();
	Difference.Z = 0;
	float Mag = Difference.Size();
	Difference.Normalize();
	MoveCapsule(Capsule, Difference, Mag, false, true);
}

void UVRCharacterComponentNew::CameraCollisionCheck()
{
	if (!Capsule || !Camera || !CameraOrigin || !OwningPawn)
		return;

	FHitResult Hit;
	FVector StartLocation = Capsule->GetComponentLocation();
	StartLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
	FVector EndLocation = Camera->GetComponentLocation();
	SphereCast(Hit, StartLocation, EndLocation, CameraCollisionSphereRadius, bShowDebugMovement, FColor::Blue, 10.f);

	if (Hit.bBlockingHit)
	{
		if (SnapCameraBackWhenInWrongLocation)
		{
			XYRecenter();
			//ZRecenter();
		}
		else
		{
			FVector Offset = Hit.Location - Camera->GetComponentLocation();
			Offset.Z = 0;
			CameraOrigin->AddWorldOffset(Offset);
		}

	}
}

void UVRCharacterComponentNew::HmdCollisionDistanceCheck()
{
	if (!Capsule || !Camera || !CameraOrigin)
		return;

	FVector StartLocation = Capsule->GetComponentLocation();
	FVector EndLocation = Camera->GetComponentLocation();
	StartLocation.Z = EndLocation.Z;

	float Distance = FVector::Distance(StartLocation, EndLocation);
	if (Distance > MaxCameraDistanceFromCollision)
	{
		if (SnapCameraBackWhenInWrongLocation)
		{
			XYRecenter();
		}
		else
		{
			FVector Dir = StartLocation - EndLocation;
			Dir.Normalize();

			float Offset = Distance - MaxCameraDistanceFromCollision;
			CameraOrigin->AddWorldOffset(Dir * Offset);

		}
	}
}

void UVRCharacterComponentNew::ScaleCapsuleHeight(float CameraZ)
{
	if (!Capsule || !Camera || !CameraOrigin)
		return;

	/*Save old collision Hight for later*/
	float OldHeight = Capsule->GetScaledCapsuleHalfHeight();

	/*Work out new collision height*/
	//float CameraZ = Camera->GetComponentLocation().Z;
	float ActorZ = CameraOrigin->GetComponentLocation().Z;
	float NewCapHeight = (CameraZ - ActorZ) / 2;
	Capsule->SetCapsuleHalfHeight(NewCapHeight);

	FVector CapLoc = Capsule->GetComponentLocation();
	CapLoc.Z = CameraOrigin->GetComponentLocation().Z + Capsule->GetScaledCapsuleHalfHeight();
	CapLoc.Z += SMALL_NUMBER;
	Capsule->SetWorldLocation(CapLoc);
}

FVector2D UVRCharacterComponentNew::ConsumeMovementVector()
{
	FVector2D V = MovementVectorToConsume;
	MovementVectorToConsume = FVector2D::ZeroVector;
	return V;
}

float UVRCharacterComponentNew::ConsumeYawInput()
{
	float Yaw = YawInputToConsume;
	YawInputToConsume = 0;
	return Yaw;
}

void UVRCharacterComponentNew::XYRecenter()
{
	if (!Camera || !CameraOrigin || !Capsule || !OwningPawn)
		return;

	FVector MoveOffset;
	MoveOffset.X = Capsule->GetComponentLocation().X - Camera->GetComponentLocation().X;
	MoveOffset.Y = Capsule->GetComponentLocation().Y - Camera->GetComponentLocation().Y;
	MoveOffset.Z = 0;

	FVector NewLocation = CameraOrigin->GetComponentLocation() + MoveOffset;
	CameraOrigin->SetWorldLocation(NewLocation);
}

void UVRCharacterComponentNew::ZRecenter()
{
	if (!Camera || !CameraOrigin || !Capsule || !OwningPawn)
		return;

	FVector MoveOffset;
	MoveOffset.X = 0;
	MoveOffset.Y = 0;
	MoveOffset.Z = (Capsule->GetComponentLocation().Z - Capsule->GetScaledCapsuleHalfHeight())
		- CameraOrigin->GetComponentLocation().Z;

	CameraOrigin->AddWorldOffset(MoveOffset);
}

void UVRCharacterComponentNew::OwningPlayerHandler(float DeltaTime)
{
	ScaleCapsuleHeight(Camera->GetComponentLocation().Z);

	MoveCollisionToHmd();

	HandleRotation(DeltaTime);

	switch (CurrentMovementType)
	{
	case EMovementType::EMM_Grounded:
		HandleGroundedMode(DeltaTime);
		break;
	case EMovementType::EMM_Falling:
		HandleFallingMode(DeltaTime);
		break;
	default:
		break;
	}

	HmdCollisionDistanceCheck();

	CameraCollisionCheck();

	
}

void UVRCharacterComponentNew::SimProxyHandler(float DeltaTime)
{
	switch (CurrentMovementType)
	{
	case EMovementType::EMM_Grounded:
		HandleGroundedMode(DeltaTime);
		break;
	case EMovementType::EMM_Falling:
		HandleFallingMode(DeltaTime);
		break;
	default:
		break;
	}

}

void UVRCharacterComponentNew::AuthorativeHandler(float DeltaTime)
{

}

void UVRCharacterComponentNew::HandleRotation(float DeltaTime)
{
	float YawInput = ConsumeYawInput();

	if ((YawInput < RotationalDeadZone && YawInput > -RotationalDeadZone) || YawInput == 0)
	{
		if (!bCanSnapTurn)
			bCanSnapTurn = true;

		return;
	}

	if (bUseSmoothTurning)
	{
		YawRotation(SmoothTurningSensitivity * YawInput * DeltaTime);
	}
	else
	{
		if (bCanSnapTurn)
		{
			if (YawInput > 0)
				YawRotation(SnapTurningAmount);
			else
				YawRotation(-SnapTurningAmount);

			bCanSnapTurn = false;
		}
	}
}

void UVRCharacterComponentNew::YawRotation(float Amount)
{
	if (!Camera || !CameraOrigin)
		return;

	//New Rotation
	FVector Distance = CameraOrigin->GetComponentLocation() - Camera->GetComponentLocation();
	FVector Rotation = Distance.RotateAngleAxis(Amount, FVector(0, 0, 1));
	FVector FinalLocation = Camera->GetComponentLocation() + Rotation;

	CameraOrigin->SetWorldLocation(FinalLocation);
	CameraOrigin->AddRelativeRotation(FRotator(0, Amount, 0));
}

void UVRCharacterComponentNew::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRCharacterComponentNew, SyncedHMDLocation);
}
