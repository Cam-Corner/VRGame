// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Guns/GunMagazine.h"
#include "Components/BoxComponent.h"
#include "Items/Guns/VRGun.h"
#include "Items/Guns/AutoLoadingGun.h"

AGunMagazine::AGunMagazine()
{
	SetActorTickEnabled(true);
	ReloadHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("ReloadHitbox"));
	ReloadHitbox->SetupAttachment(GetRootComponent());
	ReloadHitbox->ComponentTags.Add("Magazine");
}

void AGunMagazine::BeginPlay()
{
	Super::BeginPlay();

	AmmoLeftInMag = DefaultMaxMagazineSize;
}

void AGunMagazine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentDelay > 0)
		CurrentDelay -= DeltaTime;
}

/* Attach To Gun */
bool AGunMagazine::AttachToGun(AVRGun* Gun, FVector MagLocalOffset)
{
	if (CurrentDelay <= 0)
	{
		MainHandReleased();
		//DropItemFromHand();
		//ItemBaseMesh->SetEnableGravity(false);
		ItemBaseMesh->SetSimulatePhysics(false);
		AttachToComponent(Gun->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		SetActorRotation(Gun->GetActorRotation());
		//SetActorLocation(Gun->GetActorLocation());
		SetActorRelativeLocation(MagLocalOffset);
		//AddActorLocalOffset(MagLocalOffset * Gun->GetActorForwardVector());
		GunAttachedTo = Gun;
		bInsideGun = true;

		return true;
	}
	
	return false;
}

/* Attach To Gun */
void AGunMagazine::DetachFromGun()
{
	//ItemBaseMesh->SetEnableGravity(true);
	ItemBaseMesh->SetSimulatePhysics(true);
	//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Weapon Released", true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	GunAttachedTo = NULL;
	bInsideGun = false;

	CurrentDelay = 1;
}

bool AGunMagazine::CanGrabItem()
{
	/*if (bInsideGun)
		return false;*/

	if (GunAttachedTo)
	{
		if (AAutoLoadingGun* Gun = Cast<AAutoLoadingGun>(GunAttachedTo))
		{
			if (!Gun->CanRemoveMagWithHands())
				return false;
		}
	}

	return Super::CanGrabItem();
}

bool AGunMagazine::MainHandGrabbed(AVRHand* Hand)
{
	if (GunAttachedTo)
	{
		if (AAutoLoadingGun* Gun = Cast<AAutoLoadingGun>(GunAttachedTo))
		{
			Gun->RemovedMagazineFromGun();
			DetachFromGun();
		}
	}

	return Super::MainHandGrabbed(Hand);
}

