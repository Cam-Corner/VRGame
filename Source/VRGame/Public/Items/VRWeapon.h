// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/VRItem.h"
#include "VRWeapon.generated.h"


UENUM()
enum eWeaponCategory
{
	EWC_Uncategorised UMETA(DisplayName = "Uncategorised"),
	EWC_Pistol UMETA(DisplayName = "Pistol"),
	EWC_SMG UMETA(DisplayName = "SMG"),
	EWC_DMR UMETA(DisplayName = "DMR"),
	EWC_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWC_Shotgun UMETA(DisplayName = "Shotgun"),
	EWC_LMG UMETA(DisplayName = "LMG"),
	EWC_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWC_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWC_Other UMETA(DisplayName = "Other"),
};

UCLASS()
class VRGAME_API AVRWeapon : public AVRItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		TEnumAsByte<eWeaponCategory> WeaponCategory;


};
