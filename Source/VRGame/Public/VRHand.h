// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "VRHand.generated.h"

USTRUCT()
struct FItemComponent
{
	GENERATED_BODY()

	FName ItemPartGrabbed = "NotNamed";
	UPrimitiveComponent* PartGrabbed;
	class AVRItem* PartGrabbedItem;
};


UCLASS()
class VRGAME_API AVRHand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRHand();


	void GripPressed(float Value);
	void TriggerPressed(float Value);
	void BottomButtonPressed();
	void TopButtonPressed();

	FVector GetMotionControllerLocation()
	{
		return MotionController->GetComponentLocation();
	}

	FRotator GetMotionControllerRotation()
	{
		return MotionController->GetComponentRotation();
	}

	void DropItemsInHand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
	Set the motion controller source
	@"Left" for left hand
	@"Right" for right hand
	*/
	void SetMotionSource(FName Source)
	{
		if (MotionController != NULL)
			MotionController->MotionSource = Source;
			
		if (WidgetIC != NULL && Source == "Left")
			WidgetIC->PointerIndex = 1.0f;
	}

protected:
	/* The mesh that the hand will use */
	UPROPERTY(VisibleAnywhere, Category = "HandMesh")
		class UStaticMeshComponent* HandMesh;

	/* The Collision mesh of the hand */
	UPROPERTY(VisibleAnywhere, Category = "HandMesh")
		class USphereComponent* HandMeshCol;

private:
	/** function that is called when the hand overlaps something */
	UFUNCTION() 
	void HandGrabSphereOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
													class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
													const FHitResult& HitResult);

	/** function that is called when the hand leaves an overlaped object */
	UFUNCTION() 
	void HandGrabSphereOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
													  class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void UWInteractPressed();

	void UWInteractReleased();

	/** The Motion Controller  */
	class UMotionControllerComponent* MotionController;

	TArray<class AVRItem*> OverlappedItems;

	TArray<FItemComponent> OverlappedItemComponents;

	class AVRItem* ItemInHand = NULL;

	class USceneComponent* RootComp;

	FItemComponent ItemComponentInHand;

	bool bBeingHeld = false;

	UWidgetInteractionComponent* WidgetIC;

	void CheckForUIHits();
};
