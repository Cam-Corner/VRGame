// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRTrackingHands.generated.h"

class USkeletalMeshComponent;
class UMotionControllerComponent;
class USceneComponent;
class AVRPhysicsHand;
class USphereComponent;
class AVRItem;

UCLASS()
class VRGAME_API AVRTrackingHands : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRTrackingHands();

	/** Set the tracking the one of the controllers */
	void SetTrackingHand(FName Hand);

	/** Get the skeletal mesh of this actor */
	USkeletalMeshComponent* GetHandSkeletalMesh() { return HandSK; }

	void GripPressed(float Value);

	void TriggerPressed(float Value);

	void BottomButtonPressed();

	void TopButtonPressed();

	UMotionControllerComponent* GetMotionController()
	{
		return MotionController;
	}

	bool SpawnPhysicsHands(AActor* NewOwner);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/* This is the root component of the hands and is the origin point of hand tracking */
	USceneComponent* RootComp;

	/* This is the motion controller component that hands tracking data from the controllers */
	UMotionControllerComponent* MotionController;

	/* This the the hands of the character, they dont have any collision and pass through everything */
	USkeletalMeshComponent* HandSK;

	/** Physics hand attached to this actor */
	UPROPERTY(Replicated)
	AVRPhysicsHand* PhysicsHand;

	UPROPERTY(Replicated)
		FVector HandLoc = FVector::ZeroVector;

	UPROPERTY(Replicated)
		FRotator HandRot = FRotator::ZeroRotator;

	UFUNCTION(Server, Reliable)
		void Server_LocAndRot(FVector Loc, FRotator Rot);

	UPROPERTY(EditAnywhere, Category = "VR Hands")
	TSubclassOf<AVRPhysicsHand> PhysicsHandsToSpawn;


	UPROPERTY(EditAnywhere, Category = "VR Hands")
		bool bHideHands = false;

	FVector OldRoot;

	bool bCheckTracking = true;
};
