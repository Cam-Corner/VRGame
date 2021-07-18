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
	HandleMovementNonPhysics(DeltaTime);
	SmoothRotation(DeltaTime);

	// ...
}

void UVRCharacterComponent::XYRecenter()
{
	if (!VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterCapsule)
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
	if (!VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterCapsule)
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
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterForwardVector)
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
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterForwardVector)
		return;

	/*Set the body Collision rotation to 0, 0, 0 to stop it from falling over*/
	//VRCharacterCapsule->SetWorldRotation(FRotator(0, 0, 0));

	FVector FinalVelocity = XYMovement;
	XYMovement = FVector(0, 0, 0);
	/*FinalVelocity.Z = VRCharacterCapsule->GetComponentVelocity().Z;

	VRCharacterCapsule->ComponentVelocity = FinalVelocity;*/

	/*Check if grounded and if not then apply gravity*/
	{
		FHitResult FloorHit;
		FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
		float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
		FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 5));
		FCollisionShape SphereTrace;
		FCollisionQueryParams SphereParams;
		SphereParams.AddIgnoredActor(GetOwner());

		SphereTrace.SetSphere(SphereRadius);

		bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
			VRCharacterCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Visibility,
			SphereTrace, SphereParams);

		if (!bHitFloor)
		{
			//FinalVelocity.Z = -GravitySpeed;
			GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
		}

	}

	float VelocityMag = VRCharacterCapsule->GetComponentVelocity().Size();
	FVector VelDir = VRCharacterCapsule->GetComponentVelocity();

	//if (LastVelocityDir != VelocityDirection)
	/*{
		float Difference = VelocityMag;
		VRCharacterCapsule->AddImpulse(FinalVelocity, NAME_None, true);
		GEngine->AddOnScreenDebugMessage(5, 0.1f, FColor::Yellow, "Dir Changed", true);
	}*/

	if (VelocityDirection.IsZero())
	{
		if (VelocityMag != 0)
		{		
			float Difference = VelocityMag;
			VRCharacterCapsule->AddImpulse(-VelDir, NAME_None, true);
			GEngine->AddOnScreenDebugMessage(2, 0.1f, FColor::Yellow, "Stopping", true);
		}
	}
	else
	{
		//forward & backwards impulse force
		{		
			VRCharacterCapsule->AddImpulse(VRCharacterCapsule->GetForwardVector() * (WalkMovementSpeed * VelocityDirection.Y));
			VRCharacterCapsule->AddTorque(FVector(0, 0, 500));
		}

		// Left & right impulse force
		{
			//VRCharacterCapsule->AddImpulse(RightVelDirection * (WalkMovementSpeed * VelocityDirection.X));
		}
	
		/*if (ForwardVelDirection != LastForwardVelDirection || RightVelDirection != LastRightVelDirection)
		{
			VelocityMag = VRCharacterCapsule->GetComponentVelocity().Size();
			FVector NewDir = (ForwardVelDirection + RightVelDirection);
			NewDir.Normalize();

			VRCharacterCapsule->AddImpulse(NewDir * VelocityMag, NAME_None, true);
		}
		*/

		 if (VelocityMag > WalkMovementSpeed)
		 {		
			VelocityMag = VRCharacterCapsule->GetComponentVelocity().Size();
			float Difference = VelocityMag - WalkMovementSpeed;
			VelDir = VRCharacterCapsule->GetComponentVelocity();
			VelDir.Normalize();
			VRCharacterCapsule->AddImpulse(VelDir * -Difference);
		 }

	}
	//GEngine->AddOnScreenDebugMessage(2, 0.1f, FColor::Yellow, VRCharacterCapsule->GetComponentVelocity().ToCompactString(), true);

	if (VelocityDirection.X != 0 || VelocityDirection.Y != 0)
		XYRecenter();

	if (VelocityDirection.Z != 0)
		ZRecenter();


	LastVelocityDir = VelocityDirection;
	LastForwardVelDirection = ForwardVelDirection;
	LastRightVelDirection = RightVelDirection;
}

void UVRCharacterComponent::HandleMovementNonPhysics(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin || !VRCharacterForwardVector)
		return;

	/*Set the forward rotation to the Yaw of the camera*/
	FRotator NewRotation = VRCharacterCapsule->GetComponentRotation();
	NewRotation.Yaw = VRCharacterCamera->GetComponentRotation().Yaw;
	VRCharacterCapsule->SetWorldRotation(NewRotation);

	//Check for the floor and apply gravity if needed
	{
		FHitResult FloorHit;
		FVector StartLocation = VRCharacterCapsule->GetComponentLocation();
		float SphereRadius = VRCharacterCapsule->GetScaledCapsuleRadius() - 2;
		FVector EndLocation = StartLocation - FVector(0, 0, (VRCharacterCapsule->GetScaledCapsuleHalfHeight() + 5));
		FCollisionShape SphereTrace;
		FCollisionQueryParams SphereParams;
		SphereParams.AddIgnoredActor(GetOwner());

		SphereTrace.SetSphere(SphereRadius);

		bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
			VRCharacterCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Visibility,
			SphereTrace, SphereParams);

		if (!bHitFloor)
		{
			FVector MovetoLocation = GetOwner()->GetActorLocation() +
				(FVector(0, 0, 1) * (GravitySpeed * DeltaTime));

			bool bFoundSpot = GetWorld()->FindTeleportSpot(GetOwner(), MovetoLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				GetOwner()->SetActorLocation(MovetoLocation);
				//ZRecenter();
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Yellow, "GRAVITY", true);
			}
		}

	}

	
	//Apply the desired movement
	{
		if (!XYMovement.IsZero())
		{
			FVector MovetoLocation = GetOwner()->GetActorLocation() + (XYMovement * DeltaTime);

			bool bFoundSpot = GetWorld()->FindTeleportSpot(GetOwner(), MovetoLocation, VRCharacterCapsule->GetComponentRotation());

			if (bFoundSpot)
			{
				GetOwner()->SetActorLocation(MovetoLocation);
				XYRecenter();
			}
		}
	}
}

void UVRCharacterComponent::SmoothRotation(float DeltaTime)
{
	if (!VRCharacterCapsule || !VRCharacterCamera || !VRCharacterCameraOrigin)
		return;

	//set a deadzone for the thumbsticks
	float ThumbstickDeadZone = 0.75f;
	
	if (bSmoothTurning)
	{
		if (CurrentXRotationValue > ThumbstickDeadZone || CurrentXRotationValue < -ThumbstickDeadZone)
		{
			if (CurrentXRotationValue != 0)
			{
				//New Rotation
				FVector Distance = GetOwner()->GetActorLocation() - VRCharacterCamera->GetComponentLocation();
				FVector Rotation = Distance.RotateAngleAxis((SmoothTurningSensitivity * CurrentXRotationValue) * DeltaTime, FVector(0, 0, 1));
				FVector FinalLocation = GetOwner()->GetActorLocation() + Rotation;

				//VRCharacterCameraOrigin->SetWorldLocation(FinalLocation);
				//VRCharacterCameraOrigin->SetWorldRotation(FRotator(0, (SmoothTurningSensitivity * CurrentXRotationValue) * DeltaTime, 0));
				GetOwner()->SetActorLocation(FinalLocation);
				GetOwner()->AddActorWorldRotation(FRotator(0, SmoothTurningSensitivity * DeltaTime, 0));
			}
		}
	}
	else
	{
		if (!bDoneSnapTurning)
		{
			if (CurrentXRotationValue > ThumbstickDeadZone || CurrentXRotationValue < -ThumbstickDeadZone)
			{
				if (CurrentXRotationValue != 0)
				{
					//New Rotation
					FVector Distance = VRCharacterCapsule->GetComponentLocation() - VRCharacterCamera->GetComponentLocation();
					FVector Rotation = Distance.RotateAngleAxis((SnapTurningAmount * CurrentXRotationValue) * DeltaTime, FVector(0, 0, 1));
					FVector FinalLocation = VRCharacterCamera->GetComponentLocation() + Rotation;

					VRCharacterCameraOrigin->SetWorldLocation(FinalLocation);
					VRCharacterCameraOrigin->SetWorldRotation(FRotator(0, (SnapTurningAmount * CurrentXRotationValue) * DeltaTime, 0));
					bDoneSnapTurning = true;
				}
			}
		}
		else if (CurrentXRotationValue < ThumbstickDeadZone || CurrentXRotationValue > -ThumbstickDeadZone)
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

