// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRItem.h"
#include "Items/Guns/VRGun.h"
#include "GunMagazine.generated.h"


class UBoxComponent;
/**
 * 
 */
UCLASS()
class VRGAME_API AGunMagazine : public AVRItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGunMagazine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* Get how much ammo is left in the magazine */
	unsigned int GetAmmoLeft() { return AmmoLeftInMag; }

	/* Set how much ammo is left in the magazine */
	void SetAmmoLeft(unsigned int Amount) { AmmoLeftInMag = Amount; }

	/* Remove a bullet from the magazine */
	void RemoveABullet() { if (AmmoLeftInMag > 0) AmmoLeftInMag -= 1; }

	/* Attach To Gun */
	bool AttachToGun(AVRGun* Gun, FVector MagLocalOffset);

	/* Attach To Gun */
	void DetachFromGun();

	virtual bool CanGrabItem() override;

	/* Needs to be called when a player grabs a weapon */
	virtual bool MainHandGrabbed(AVRHand* Hand) override;

protected:
	/* The size of the magazine
	@ The max amount of ammo it can store
	*/
	UPROPERTY(EditAnywhere, Category = "Magazine Settings")
		unsigned int DefaultMaxMagazineSize = 10;

private:
	/* current ammount of ammo left in the magazine */
	unsigned int AmmoLeftInMag = 10;

	/* Mag currently inside a gun */
	bool bInsideGun = false;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* ReloadHitbox;


	AVRGun* GunAttachedTo;

	float CurrentDelay = 0;
};
