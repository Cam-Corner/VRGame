// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRCharacterComponent.generated.h"

class UCameraComponent;
class UCapsuleComponent;
class USceneComponent;

UENUM()
enum EMovementModes
{
	EMM_SlowWalk  UMETA(DisplayName = "SlowWalk"),
	EMM_Walk  UMETA(DisplayName = "Walk"),
	EMM_Run  UMETA(DisplayName = "Run"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRGAME_API UVRCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRCharacterComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

/*======
Public Functions
=======*/
public:
	/*Recenter the player camera to the collision box on the XY plane*/
	void XYRecenter();

	/*Recenter the player camera to the collision box on the Z plane*/
	void ZRecenter();

	/* Set XY Movement Direction */
	void SetXYMovementDirection(FVector2D Dir);
	
	/* Used to set the current movement mode */
	void SetMovementMode(EMovementModes Mode) { CurrentMovementMode = Mode; }

	/* Set the camera component this actor component should use */
	void SetVRCharacterCamera(UCameraComponent* Camera, USceneComponent* CameraOrigin) 
	{
		VRCharacterCamera = Camera; 
		VRCharacterCameraOrigin = CameraOrigin;
	}

	/* Set the capsule component this actor component should use */
	void SetVRCharacterCapsule(UCapsuleComponent* Capsule) 
	{ 
		VRCharacterCapsule = Capsule; 
	}

	/* Set the Pawn that owns this component */
	void SetComponentOwner(APawn* Owner)
	{
		ComponentOwner = Owner;
	}

	/* Set the capsule component this actor component should use */
	void SetXRotationValue(float Value) { CurrentXRotationValue = Value; }

/*======
Private Functions
=======*/
private:
	/*Handles the final movement of the character*/
	void HandleMovement(float DeltaTime);

	/* Handle Smooth Rotation */
	void SmoothRotation(float DeltaTime);
	
	/* Scale the players collision from the floor to the headset*/
	void ScaleCollisionWithPlayer();

	/* Move Collision To HMD headset */

	void MoveCollisionToHMD();

	/* check how far the HMD is from the collision */
	void CheckHMDDistanceFromCollision();


/*=======
Private UPROPERTY() Variables
=========*/
private:
	/* The slow walk movement speed */
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
		float SlowWalkMovementSpeed = 250;

	/* The walk movement speed */
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
		float WalkMovementSpeed = 600;

	/* The Run movement speed */
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
		float RunMovementSpeed = 1000;

	/* The Gravity speed */
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
		float GravitySpeed = 600;


	/* set a deadzone for the thumbsticks */
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float RotationThumbstickDeadZone = 0.25f;

	/* Speed of the smooth camera turning */
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		float SmoothTurningSensitivity = 15;

	/* Snap turning amount */
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		float SnapTurningAmount = 45;

	/* if true we use smooth turning || if false we use snap turning */
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		bool bSmoothTurning = true;

	/* Max Distance the camera can be from the collision */
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
		float MaxCameraDistanceFromCollision = 20.0f;

/*======
Private Components
=======*/
private:
	/* The current Movement mode the character is in */
	TEnumAsByte<EMovementModes> CurrentMovementMode;

	/* The camera component set by the parent actor to use */
	UCameraComponent* VRCharacterCamera;

	/* The camera origin point set by the parent actor */
	USceneComponent* VRCharacterCameraOrigin;

	/* The capsule component set by the parent actor to use */
	UCapsuleComponent* VRCharacterCapsule;

	/* The owner of this component */
	APawn* ComponentOwner;
/*======
Private Variables
=======*/
private:

	/* The final velocity to add */
	FVector XYMovement = { 0, 0, 0 };
	
	/* velocity Direction */
	FVector VelocityDirection = { 0, 0, 0 };

	/* velocity Direction */
	FVector ForwardVelDirection = { 0, 0, 0 };
	FVector LastForwardVelDirection = { 0, 0, 0 };
	
	/* velocity Direction */
	FVector RightVelDirection = { 0, 0, 0 };
	FVector LastRightVelDirection = { 0, 0, 0 };

	/* velocity Directoin */
	FVector LastVelocityDir = { 0, 0, 0 };

	/* The final velocity to add */
	float CurrentXRotationValue = 0;

	/* check if the already applied the snap turning value */
	float bDoneSnapTurning = false;
};
