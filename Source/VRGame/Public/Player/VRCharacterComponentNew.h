// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRCharacterComponentNew.generated.h"

class UCapsuleComponent;

UENUM(BlueprintType)
enum EMovementType
{
	EMM_Grounded	UMETA(DisplayName = "Grounded"),
	EMM_Falling		UMETA(DisplayName = "Falling"),
	EMM_Flying		UMETA(DisplayName = "Flying"),
	EMM_Swimming	UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum EGroundedType
{
	EGM_Walk UMETA(DisplayName = "Walk"),
	EGM_Jog UMETA(DisplayName = "Jog"),
	EGM_Sprint UMETA(DisplayName = "Sprint"),
	
};

/*USTRUCT()
struct FPlayerMove
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Dir = FVector::ZeroVector;

};*/



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRGAME_API UVRCharacterComponentNew : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRCharacterComponentNew();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

/* ================
*  Non networking Functions
*  ================ */
public:
	/** Add a direction to the input, the vector passed should be normalized */
	void AddMovementVector(const FVector Dir) { MovementVectorToConsume += Dir; }

	/** Add Yaw Input, if 1, then we use the full sensitivty value, if 0.5f then we use half etc */
	void AddPitchInput(const float Amount) { YawInputToConsume += Amount; }

protected:
	/** Moves a specified capsule in the direction givent by the amount given */
	void MoveCapsule(UCapsuleComponent* CapToMove, const FVector Dir,
		const float Amount, const bool bXYRecenter, bool bZRecenter);

	/** Consume the input vector */
	FVector ConsumeMovementVector();

	/** Consume the yaw input */
	FVector ConsumeYawInput();

	/** Recented the HMD on the XY Axis */
	void XYRecenter();

	/** Recented the HMD on the Z Axis */
	void ZRecenter();

	/** Handles the owning player movement */
	void OwningPlayerHandler(float DeltaTime);

	/** Handles the simulated proxy syncing */
	void SimProxyHandler(float DeltaTime);

	/** Handles authorative functions/code */
	void AuthorativeHandler(float DeltaTime);

	/** Called to smooth turn the player */
	void SmoothTurning(float DeltaTime);

	/** Called to snap turn the player */
	void SnapTurning(float DeltaTime);

	/** Moves the collision towards the HMD */
	void MoveCollisionToHMD();

/* ================
*  Networking Functions
*  ================ */
private:
	/** Send move to the server */
	/*UFUNCTION(Server, Reliable)
		void Server_SendMove(FPlayerMove& Move);*/

/* ================
*  Non visible Variables
*  ================ */
private:
	UPROPERTY(Replicated)
		FVector SyncedHMDLocation = FVector::ZeroVector;

	FVector MovementVectorToConsume = FVector::ZeroVector;
	float YawInputToConsume = 0.0f;

/* ==========================================
*  Below are the characters visible variables
*  ========================================== */
protected:
	/* =================
	*  General Settings
	*  ================= */
	/* Default Movement Type */
	UPROPERTY(EditAnywhere, Category = "Character Component: General Settings")
		TEnumAsByte<EMovementType> DefaultMovementType = EMovementType::EMM_Grounded;
	/* Default Grounded Type */
	UPROPERTY(EditAnywhere, Category = "Character Component: General Settings")
		TEnumAsByte<EGroundedType> DefaultGroundedType = EGroundedType::EGM_Walk;

	/* =================
	*  Grounded Settings
	*  ================= */

	UPROPERTY(EditAnywhere, Category = "Character Component: Grounded Settings")
		float DefaultWalkSpeed = 250;

	UPROPERTY(EditAnywhere, Category = "Character Component: Grounded Settings")
		float DefaultJogSpeed = 350;

	UPROPERTY(EditAnywhere, Category = "Character Component: Grounded Settings")
		float DefaultSprintSpeed = 500;

	/** If true we toggle the jog/sprint, if false you hold to jog/sprint*/
	UPROPERTY(EditAnywhere, Category = "Character Component: Grounded Settings")
		bool bSprintJogToggle = false;

	/** If true we toggle the jog/sprint, if false you hold to jog/sprint*/
	UPROPERTY(EditAnywhere, Category = "Character Component: Grounded Settings",
		meta = (ClampMin = "0", ClampMax = "90", UIMin = "0", UIMax = "90"))
		float MaxWalkableFloorAnge = 45.f;

	/* =================
	*  Falling Settings
	*  ================= */

	UPROPERTY(EditAnywhere, Category = "Character Component: Falling Settings")
		float GravitySpeed = 250;

	/** how fast can the player move while in the air/falling */
	UPROPERTY(EditAnywhere, Category = "Character Component: Falling Settings")
		float AirControl = 50.f;

	/* =================
	*  Flying Settings
	*  ================= */


	/* =================
	*  Swimming Settings
	*  ================= */

	/* =================
	*  Camera Settings
	*  ================= */

	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		bool bUseSmoothTurning = false;

	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		float SmoothTurningSensitivity = 15;

	/** how much (in Degrees) will the camera turn per snap */
	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		float SnapTurningAmount = 15;

	/** how far can the players HMD/Head get away from the top of the movement collision */
	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		float MaxCameraDistanceFromCollision = 20.0f;

	/** what is the radius of the sphere that we use to sweep to the HMD to see if we are colliding with anything */
	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		float CameraCollisionSphereRadius = 20.0f;
	
	/* If true we recenter the camera back to the collision capsule when the camera is in an unsutable location
	* If false then we just move the camera back to a location that is sutable
	*/
	UPROPERTY(EditAnywhere, Category = "Character Component: Camera Settings")
		bool SnapCameraBackWhenInWrongLocation = false;

	/* =================
	*  DeadZone Settings
	*  ================= */

	UPROPERTY(EditAnywhere, Category = "Character Component: DeadZone Settings")
		float MovementDeadZone = .5f;

	UPROPERTY(EditAnywhere, Category = "Character Component: DeadZone Settings")
		float RotationalDeadZone = .5f;

	/* =================
	*  Network Settings
	*  ================= */

	UPROPERTY(EditAnywhere, Category = "Character Component: Network Settings")
		float MaxLocationError = 20.f;

};
