// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Guns/VRGun.h"
#include "AutoLoadingGun.generated.h"

class AGunMagazine;
class UAudioComponent;
class USoundCue;

UENUM()
enum eGunFireMode
{
	EGFM_Other UMETA(DisplayName = "Other"),
	EGFM_FullyAutomaticMode UMETA(DisplayName = "Fully Automatic Mode"),
	EGFM_SemiAutomaticMode UMETA(DisplayName = "Semi-Automatic Mode"),
	EGFM_BurstModeMode UMETA(DisplayName = "Burst Mode")
};


USTRUCT()
struct FGunFireModes
{
	GENERATED_BODY()

	/* Can the weapon be shot in automatic mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bAutomaticModeAllowed = false;

	/* Rotation Of the fire mode mesh when in automatic mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float RotationOfAutoSwitch = 0;

	/* Can the weapon be shot in semi-automatic mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bSemiAutomaticModeAllowed = false;

	/* Rotation Of the fire mode mesh when in SemiAutomatic mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float RotationOfSemiAutoSwitch = 0;

	/* Can the weapon be shot in burst mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bBurstModeAllowed = false;

	/* Rotation Of the fire mode mesh when in burst mode */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float RotationOfBurstSwitch = 0;

	/* If the weapon can be shot in burst mode, how many shots will fire per burst*/
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float ShotsPerBurst = 3;
};

/**
 * 
 */
UCLASS()
class VRGAME_API AAutoLoadingGun : public AVRGun
{
	GENERATED_BODY()
	
public:
	AAutoLoadingGun();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPressed() override;

	virtual void BottomButtonPressed() override;

	virtual void TopButtonPressed() override;

	virtual void OffHandGrabbed(AVRPhysicsHand* Hand, const FName& PartNameGrabbed) override;

	virtual void OffHandReleased() override;

	void RemovedMagazineFromGun();

	bool CanRemoveMagWithHands() { return bCanRemoveMagWithHands; }

protected:
	/*==============
	Gun Settings
	================*/
	/* Gun Fire Mode */
	TEnumAsByte<eGunFireMode> eCurrentFireMode;

	/* If the gun requires 2 Hands to aim properly */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bTwoHandedGun = false;

	/* Setup which gunfire modes this weapon can have */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		FGunFireModes GunFireModesAllowed;

	/* How many shots the gun can fire per Minute */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float RoundsPerMinute = 0;

	/* Can Eject the magazine by pressing a button on the controller */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bQuickMagEject = false;

	/* Can the magazine be removed from the gun using your hands */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bCanRemoveMagWithHands = false;

	/* How Much the slider moves when shooter */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float SliderShootingOffset { 0 };

	/* How Much the slider needs to move to reload */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float ReloadSliderOffset { 0 };

	/* Position of the slider when their isn't any bullets in the chamber */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		float GunEmptySliderOffset { 0 };

	/* The Slider that moves/is used when reloading the weapon
	*@ This can be things like:
	* Barrel of a gun
	* Reciever of a gun etc
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		class UStaticMeshComponent* ReloadPartMesh;

	/* Does the slider used for reloading move when shooting */
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		bool bReloadPartMovesWhenShooting = false;

	/* The Slider that moves about when being fired
	* This should only be used if the gun reciever moves independently from the reload part
	* for example a part of a reciever might move when shooting but the reload part doesnt
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	class UStaticMeshComponent* ShootingPartMesh;

	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
		class UBoxComponent* SliderGrabCollisionLocation;

	/*==============
	Magazine Settings
	================*/
	/* What class is the magazine */
	UPROPERTY(EditAnywhere, Category = "Magazine Settings")
		TSubclassOf<AGunMagazine> DefaultGunMagazine;

	/* Loaded Magazine offset */
	UPROPERTY(EditAnywhere, Category = "Magazine Settings")
		FVector LoadedMagazineOffset = { 0, 0, 0 };

	/* Does this gun use shells instead of a single magazine */
	UPROPERTY(EditAnywhere, Category = "Magazine Settings")
		bool bUsesShellsInsteadOfMagazine = false;

	/* If it uses shells, how many can this gun load */
	UPROPERTY(EditAnywhere, Category = "Magazine Settings")
		unsigned int MaxShellLoad = 7;

	/* The collision box for the magazine to be loaded */
	UPROPERTY(VisibleAnywhere, Category = "Magazine Settings")
		class UBoxComponent* ReloadCollisionLocation;

	/* The collision box for the magazine to be loaded */
	UPROPERTY(VisibleAnywhere)
		class UBoxComponent* OffHandGrabLocationAndCollision;

	UFUNCTION()
		void OnMagazineReloadOverlapEnter(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult& HitResult);

	/*=========
	* Audio Settings
	============*/
	UPROPERTY(EditAnywhere, Category = "Audio Settings")
		USoundCue* ShootingAudio;

	UPROPERTY(EditAnywhere, Category = "Audio Settings")
		USoundCue* EmptyAudio;

	UPROPERTY(EditAnywhere, Category = "Audio Settings")
		USoundCue* SliderFullyBackAudio;

private:
	/* True When the weapon can fire again */
	bool bCanFireAgain = true;

	/*
	The Default Timer that is used between shots
	@ Don't edit this, Should only be edited when working out the new value using the "rounds per minute"
	*/
	double DefaultTimeBetweenShots = 0;

	/* Timer that creates a delay between shots */
	double CurrentShotDelay = 0;

	/* Checks if the trigger (fire function been called?) has been pressed this frame */
	bool bTriggerPressedThisFrame = false;

	/* Current relative location of the slider */
	float CurrentRelativeLocation = 0;

	/* Is their a bullet in the chamber */
	bool bBulletInChamber = true;

	/* Shots left to burst fire */
	unsigned int BurstShotsLeft = 0;

	/* If it uses shells, this is how many shells are left in the gun */
	unsigned int ShellsLeft = 12;

	/* If it uses a magazine, this is the magazine that is in the weapon
	@ Can be null
	*/
	AGunMagazine* CurrentMagazine;

	/* Is the offhand holding the slider */
	bool bHoldingSlider = false;

	/* Is the offhand holding the weapon */
	bool bOffHandHoldingWeapon = false;

	/* The off hand starting location when it grabbed something */
	FVector OffHandStartingLocation{ 0, 0, 0 };

	FVector ActorLocationWhenGrabbed{ 0, 0, 0 };
	float DistanceGrabbedOnLine = 0;
	bool bGrabbedFrontSide = false;
	bool bCanEjectBullet = true;
	bool bStuckInEmptyPos = false;

private:
	void HandleWeaponShooting(float DeltaTime);
	void HandleHoldingSlider(float DeltaTime);
	void HandleSliderMovement();
	void HandleShells();
	void HandleTwoHandedWeaponAiming();

/*==========
Server Stuff
=============*/
private:
	/** Tell the server the top button was pressed */
};
