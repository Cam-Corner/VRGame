// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PIDControllers.h"
#include "Player/VRHand.h"
#include "VRPhysicsHand.generated.h"

class UBoxComponent;
class USkeletaMeshComponent;
class AVRTrackingHands;
class UPhysicalMaterial;
class UPhysicsConstraintComponent;

UCLASS()
class VRGAME_API AVRPhysicsHand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRPhysicsHand();

	/** Set Left Hand Scale */
	void SetIsLeftHand();

	/** Set following actor
	* Will most lickly be a hand/motion controllers
	*/
	void SetTrackingHands(AVRTrackingHands* Hands);

	void GripPressed(float Value);

	void TriggerPressed(float Value);

	void BottomButtonPressed();

	void TopButtonPressed();

	void DropItemsInHand();

	void NonPhysicsHandLocation();

	class UMotionControllerComponent* GetMotionController();

	const FTransform GetTrackingHandTransform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** function that is called when the hand overlaps something */
	UFUNCTION()
		void HandGrabSphereOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult& HitResult);

	/** function that is called when the hand leaves an overlaped object */
	UFUNCTION()
		void HandGrabSphereOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void EnableCollision();

	void DisableCollision();

	void PhysicsMoveCollisionToHand(float DeltaTime);

	void MovePhysicsItemToHand(float DeltaTime);

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	FCalculateCustomPhysics OnCalcCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	/**Location PID Controller Settings For This Gun*/
	UPROPERTY(EditAnywhere, Category = "Physics Tuning")
		FPDController3D LocPD;

	/**Rotation PD Controller Settings For This Gun*/
	UPROPERTY(EditAnywhere, Category = "Physics Tuning")
		FQuatPDController RotPD;

	/** The Skeletal Mesh of the hand */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandMesh")
		USkeletalMeshComponent* HandSK;

	/** physics box component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandMesh")
		UBoxComponent* PhysicsBase;


private:
	

	/* The Collision mesh of the hand */
	UPROPERTY(VisibleAnywhere, Category = "HandMesh")
		USphereComponent* HandGrabSphere;

	/** The object this hand is trying to follow / reach by physics */
	UPROPERTY(Replicated)
	AVRTrackingHands* TrackingHands;

	/** The item that is being held in the hand */
	AVRItem* ItemInHand = NULL;

	TArray<class AVRItem*> OverlappedItems;

	TArray<FItemComponent> OverlappedItemComponents;

	FItemComponent ItemComponentInHand;

	bool bBeingHeld = false;

	UPROPERTY(EditAnywhere, Category = "HandPoses")
	UAnimationAsset* IdlePose;

	UPROPERTY(EditAnywhere, Category = "HandPoses")
	UAnimationAsset* GripPose;

	//UPROPERTY(EditAnywhere, Category = "HandPoses")
		//UPhysicsConstraintComponent* PhysicsConst;

	bool bInGripPose = false;

	bool bDoOnceOnTick = false;

/*======
Server stuff
=====*/
private:
	UPROPERTY(Replicated)
		FVector PlayerHandLoc = FVector::ZeroVector;

	UPROPERTY(Replicated)
		FRotator PlayerHandRot = FRotator::ZeroRotator;

	UFUNCTION(Server, Unreliable)
		void Server_SendLocAndRot(FVector Loc, FRotator Rot);
};
