// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Player/VRHand.h"
#include "VRCharacter.generated.h"

class UVRCharacterComponent;
class AVRTrackingHands;
class AVRPhysicsHand;

UCLASS()
class VRGAME_API AVRCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Get the VRCharacter Component */
	UFUNCTION(BlueprintCallable)
	UVRCharacterComponent* GetVRCharacterComponent()
	{
		return VRCharacterComponent;
	}
protected:
	/*================
	Protected Variables
	==================*/
	UPROPERTY(EditAnywhere, Category = "PIDController") float Proportional = 0.05f;
	UPROPERTY(EditAnywhere, Category = "PIDController") float Integral = 0.0f;
	UPROPERTY(EditAnywhere, Category = "PIDController") float Derivative = 0.05f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		class UCameraComponent* VRCamera;
private:
	/*================
	Movement Variables
	==================*/
	FVector2D MovementThumbstick{ 0, 0 };

	UPROPERTY(EditAnywhere, Category = "VRCharacterComponent")
	UVRCharacterComponent* VRCharacterComponent;
private:
	/*================
	private Functions
	==================*/
	void ApplyMovement();
	
	UFUNCTION() void ForwardMovement(float Value);
	UFUNCTION() void RightMovement(float Value);
	UFUNCTION() void XRotation(float Value);
	void PIDController(const float DeltaTime, const float CurrentValue, const float DesiredValue, float& ErrorPrior, float& IntegralPrior);

	/*================
	private components
	==================*/
	class USceneComponent* RootComp;
	class USceneComponent* VRCameraRoot;

	
	UPROPERTY(VisibleAnywhere, Category = "BodyCollision")
		class UCapsuleComponent* BodyCollision;

	UPROPERTY(VisibleAnywhere, Category = "Body")
		class UStaticMeshComponent* CharacterStaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Body")
		class UStaticMeshComponent* ServerLocation;

	//class UMotionControllerComponent* LeftMotionController;
	//class UMotionControllerComponent* RightMotionController;
	//class UArrowComponent* ForwardDirection;

//===========================================
	//Temp Weapon code
	UFUNCTION() void LeftGripPressed(float Value);
	UFUNCTION() void RightGripPressed(float Value);

	UFUNCTION() void LeftTriggerPressed(float Value);
	UFUNCTION() void RightTriggerPressed(float Value);

	UFUNCTION() void LeftBottomButtonPressed();
	UFUNCTION() void LeftTopButtonPressed();

	UFUNCTION() void RightBottomButtonPressed();
	UFUNCTION() void RightTopButtonPressed();


//==========================
	//Hand Code
	UPROPERTY(EditAnywhere, Category = "VR Hands")
		TSubclassOf<class AVRTrackingHands> BP_DefaultHand;
		//TSubclassOf<class AVRHand> BP_DefaultHand;

	UPROPERTY(Replicated)
	class AVRTrackingHands* LeftHand;
	//class AVRHand* LeftHand;

	UPROPERTY(Replicated)
	class AVRTrackingHands* RightHand;
	//class AVRHand* RightHand;

	/* Spawn the vr hands */
	void SpawnHands();

	/*TArray<class AVRItem*> LeftOverlappedItems;
	TArray<class AVRItem*> RightOverlappedItems;

	AVRItem* LeftHandItem = NULL;
	AVRItem* RightHandItem = NULL;*/

//==========================
	UPROPERTY(EditAnywhere, Category = "Testing")
		bool bNonVRTesting = false;

	UFUNCTION()
		void UnPossessPawn();

	UFUNCTION(Server, Reliable)
		void Server_ClientsActorReady();

/*	UFUNCTION(Client, Reliable)
		void Client_SetLeftPhysicsHand(AVRPhysicsHand* Hand);

	UFUNCTION(Client, Reliable)
		void Client_SetRightPhysicsHand(AVRPhysicsHand* Hand);*/
};
