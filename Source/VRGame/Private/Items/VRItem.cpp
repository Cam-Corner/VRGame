// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AVRItem::AVRItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("Item Base Mesh");
	ItemBaseMesh->SetSimulatePhysics(true);
	RootComponent = ItemBaseMesh;
}

// Called when the game starts or when spawned
void AVRItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



bool AVRItem::MainHandGrabbed(AVRHand* Hand)
{
	if (bItemHeald)
		return false;

	MainHand = Hand;
	bItemHeald = true;
	//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Weapon Grabbed", true);
	//_GunBase->SetEnableGravity(false);
	ItemBaseMesh->SetSimulatePhysics(false);

	return true;
}

void AVRItem::MainHandReleased()
{
	if(MainHand)
		MainHand->DropItemsInHand();

	bShouldDropNextFrame = false;
	bItemHeald = false;
	MainHand = NULL;
	//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Weapon Released", true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//_GunBase->SetEnableGravity(true);
	ItemBaseMesh->SetSimulatePhysics(true);
}

void AVRItem::TriggerPressed()
{

}

void AVRItem::TriggerReleased()
{

}

void AVRItem::BottomButtonPressed()
{

}

void AVRItem::TopButtonPressed()
{

}

void AVRItem::OffHandGrabbed(AVRHand* Hand, const FName& PartNameGrabbed)
{
	
}

void AVRItem::OffHandReleased()
{
	OffHand = NULL;
}

bool AVRItem::CanGrabItem()
{
	if (MainHand != NULL)
		return false;

	return true;

}
