// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/VRPhysicsHand.h"
#include "Utility/PIDControllers.h"
#include "VRItem.generated.h"

class AVRPhysicsHand;

UCLASS()
class VRGAME_API AVRItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRItem();

	/** Add a force to the item */
	class UStaticMeshComponent* GetPhysicsMesh() { return ItemBaseMesh; }

	UFUNCTION(BlueprintNativeEvent)
		UPrimitiveComponent* GetMainHandGrip();

	UFUNCTION(BlueprintNativeEvent)
		UPrimitiveComponent* GetOffHandGrip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DetalTime) override;

	void SetHoldingTwoHanded(bool InbTwoHanded) { bTwoHanded = InbTwoHanded; }
public:	

	/* Called When Trigger is pressed */
	virtual void TriggerPressed();

	/* called when Trigger is released */
	virtual void TriggerReleased();

	/* Called when bottom button on the controller is pressed
	@ Will be Button "X" on left controller
	@ Will be Button "A" on right controller
	*/
	virtual void BottomButtonPressed();

	/* Called when Top button on the controller is pressed
	@ Will be Button "Y" on left controller
	@ Will be Button "B" on right controller
	*/
	virtual void TopButtonPressed();

	/* Needs to be called when a player grabs a weapon */
	virtual bool MainHandGrabbed(AVRPhysicsHand* Hand);

	/* Needs to be called when a player drops the weapon */
	void MainHandReleased();

	/* Needs to be called when you want to do something with the offhand */
	virtual void OffHandGrabbed(AVRPhysicsHand* Hand, const FName& PartNameGrabbed);

	/* Needs to be called when the offhand is released */
	virtual void OffHandReleased();

	/* Is this item being held */
	bool HoldingInHand() { return MainHand == NULL ? false : true; }

	/* Can I grab onto this item */
	virtual bool CanGrabItem();

	/* If the hand should drop this item on the next frame
	@ Useful for things such as weapon mags when they get put into the weapon
	*/
	bool ShouldDropFromHandThisFrame() 
	{ 
		if (bShouldDropNextFrame)
		{
			bShouldDropNextFrame = false;
			MainHand = NULL;
			OffHand = NULL;
			return true;
		}

		return false;
	}

	/* Tell the hand to drop this item */
	void DropItemFromHand() { bShouldDropNextFrame = true; }


private:
	/* Is the weapon already been held */
	bool bItemHeald = false;

protected:
	/* The hand the item is held in */
	UPROPERTY(Replicated)
	AVRPhysicsHand* MainHand;

	/* Off Hand */
	AVRPhysicsHand* OffHand;

	/* The weapons base mesh */
	UPROPERTY(VisibleAnywhere, Category = "Item Settings")
		class UStaticMeshComponent* ItemBaseMesh;

	/* If the hand should drop this item on the next frame
	@ Useful for things such as weapon mags when they get put into the weapon
	*/
	bool bShouldDropNextFrame = false;

	void MoveItemToHand(float DeltaTime);
	bool bTwoHanded = false;

	virtual void PhysicsTick_Implementation(float SubsetDeltaTime);
	FCalculateCustomPhysics OnCalcCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	/**Location PID Controller Settings For This Gun*/
	UPROPERTY(EditAnywhere, Category = "Physics Tuning")
		FPDController3D LocPD;

	/**Rotation PD Controller Settings For This Gun*/
	UPROPERTY(EditAnywhere, Category = "Physics Tuning")
		FQuatPDController RotPD;

/*======
Server stuff
========*/
private:
	UFUNCTION(Server, Reliable)
		void Server_PickedupItem(AVRPhysicsHand* HandToFollow);

	UFUNCTION(Client, Reliable)
		void ClientDropWeapon(AVRPhysicsHand* CurrentHandHoldingItem);

	//UPROPERTY(Replicated)
	//AVRPhysicsHand* GrabbedHand = nullptr;

};
