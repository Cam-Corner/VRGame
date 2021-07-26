// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRCharacterComponent.generated.h"

class UCameraComponent;
class UCapsuleComponent;
class USceneComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogAVRCharacterComponent, Log, All);

USTRUCT()
struct FPlayerMove
{
	GENERATED_BODY()

	float DeltaTime;
	FVector Dir;
	FVector StartLocation;
	FVector EndLocation;
};

USTRUCT()
struct FClientSide_Prediction
{
	GENERATED_BODY()
	
	FVector Dir;
	FVector LastServerLocation;
	float LastServerWorldDelta;
	bool bStillMoving;

	FString Moving = bStillMoving ? "true" : "false";

	FString ToString()
	{
		return "Dir: " + Dir.ToCompactString() + " | LastServerLoc: " + LastServerLocation.ToCompactString()
			+ " | LastServerWorldDelta: " + FString::SanitizeFloat(LastServerWorldDelta) +
			" | StillMoving: " + Moving;
	}
};

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

	UFUNCTION(BlueprintCallable)
		void SetWalkSpeed(float Value) { WalkMovementSpeed = Value; }

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
	
	/* save the users current settings */
	UFUNCTION(BlueprintCallable)
	void SaveSettings();

	/* load the usres current settings */
	void LoadSettings();

	/* save the users current settings */
	UFUNCTION(BlueprintCallable)
		void GetControlSettings(bool& GetbUsingSmoothTurning, float& GetSmoothTurningSensitivity, float& GetSnapTurningAmount)
	{
		GetbUsingSmoothTurning = bSmoothTurning;
		GetSmoothTurningSensitivity = SmoothTurningSensitivity;
		GetSnapTurningAmount = SnapTurningAmount;
	}
/*======
Functions to change default variables
=======*/
public:
	/* Set wether we should use smooth turning if true, or if false we use snap turning */
	UFUNCTION(BlueprintCallable)
	void SetShouldUseSmoothTurning(bool Value) { bSmoothTurning = Value; }

	/* set the smooth turning sensitivity */
	UFUNCTION(BlueprintCallable)
	void SetSmoothTurningSensitivity(float Value) { SmoothTurningSensitivity = Value; }

	/* set the snap turning amount */
	UFUNCTION(BlueprintCallable)
	void SetSnapTurnAmount(float Value) { SnapTurningAmount = Value; }

/*======
Private Functions
=======*/
private:
	/*Handle Networked Movement*/
	void NetworkedMovement(float DeltaTime);

	/* Movement for locally controlled pawn */
	void LocalMovement(float DeltaTime);

	/* Movement for simulated proxy */
	void SimulatedProxyMovement(float DeltaTime);

	/* Movement for simulated proxy */
	void AuthorativeMovement(float DeltaTime);

	/* Movement for simulated proxy */
	void WorkOutSimulatedProxyError();

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

	/* Checks to see if the camera is inside an object
	* It does it by doing a small sphere sweep from the top of the capsule to the camera
	* if it hits something then it means the camera isnt in a sutable location
	*/
	void CheckToSeeIfCameraIsInsideObject();


	/** Moves the player collision capsule around the world 
	* Collision is handled by sweeping the capsule by the given offet (using AddWorldOffset)
	* @param Dir	is expected to be a unit vector and should be the direction you want to move the capsule
	* @param offset is used to determind the move amount
	*/
	void MovePlayerCapsule(FVector Dir, float OffsetAmount);

	/** Gets the new direction when moving up a slope
	* @param CurrentDir should be a unit vector and is the current movement direction
	* @param SlopeDir should be a unit vector and is the slopes angle direction
	* @param RotateAxis should be a unit vector and is the axis the dir gets rotated on
	* @RETURN returns the new direction that the player should move in
	*/
	FVector GetSlopeMovementDirection(FVector CurrentDir, FVector SlopeDir, FVector RotateAxis);

	/* Apply Gravity to the player */
	void ApplyGravity(float DeltaTime);

	/* Do a sphere cast */
	void SphereCast(FHitResult& Result, FVector StartLoc, FVector EndLoc, float SphereRadius);

/*=========
Networking Functions
=======*/
public:
	UFUNCTION(Server, UnReliable)
	void Server_SendMove(FVector Dir, float DeltaTime, FVector EndLocation);

	UFUNCTION(NetMulticast, UnReliable)
	void NetMulticast_SendMove(FVector Dir,
	FVector LastServerLocation,
	float LastServerWorldDelta,
	bool bStillMoving);

	UFUNCTION(Client, Unreliable)
	void Client_SetLocation(FVector Location);

/*=======
Private UPROPERTY() Variables
=========*/
private:
	/* The slow walk movement speed */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Test Settings")
		bool bUseSweepMovement = false;

	/* The slow walk movement speed */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Movement Settings")
		float SlowWalkMovementSpeed = 250;

	/* The walk movement speed */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Movement Settings")
		float WalkMovementSpeed = 600;

	/* The Run movement speed */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Movement Settings")
		float RunMovementSpeed = 1000;

	/* The Gravity speed */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Movement Settings")
		float GravitySpeed = 600;


	/* set a deadzone for the thumbsticks */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
	float RotationThumbstickDeadZone = 0.25f;

	/* Speed of the smooth camera turning */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
		float SmoothTurningSensitivity = 15;

	/* Snap turning amount */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
		float SnapTurningAmount = 45;

	/* if true we use smooth turning || if false we use snap turning */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
		bool bSmoothTurning = true;

	/* Max Distance the camera can be from the collision */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
		float MaxCameraDistanceFromCollision = 20.0f;

	/* Max Distance the camera can be from the collision */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
	float CameraSphereCollisionRadius = 10.0f;

	/* If true we recenter the camera back to the collision capsule when the camera is in an unsutable location
	* If false then we just move the camera back to a location that is sutable
	*/
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Camera Settings")
	bool SnapCameraBackWhenInWrongLocation = false;

	/* what is the max angle the character can walk up */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Slopes and Steps")
	float MaxWalkableSlopeAngle = 45.0f;

	/* what is the max height that the character can auto step onto */
	UPROPERTY(EditAnywhere, Category = "VRCharacter: Slopes and Steps")
	float MaxStepupHeight = 45.0f;

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

	/* The the player is grounded or not */
	bool bGrounded = false;

	/* The name of the slot to save the settings */
	FString SaveSlotName{ "ControlSettingsSaveGame" };

	/* the user index to save the settings */
	uint32 UserIndex = 0;

/*===
variable only server needs
===*/
private:
	TArray<FPlayerMove> ClientsMoves;
	
	FClientSide_Prediction CurrentProxyMove;
	FClientSide_Prediction ErrorProxyMove;	

	bool bNewSimProxyUpdate = false;

	bool bSendStoppedMove = false;

	float UpdateMultiTime = 1.0f;
};
