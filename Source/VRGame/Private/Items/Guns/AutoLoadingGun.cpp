// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Guns/AutoLoadingGun.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Items/Guns/GunMagazine.h"
#include "Utility/ExtraMaths.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "VRGameInstance.h"
#include "Audio/AudioManager.h"

AAutoLoadingGun::AAutoLoadingGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ReloadPartMesh = CreateDefaultSubobject<UStaticMeshComponent>("ReloadPartMesh");
	ReloadPartMesh->SetupAttachment(GetRootComponent());

	ShootingPartMesh = CreateDefaultSubobject<UStaticMeshComponent>("ShootingPartMesh");
	ShootingPartMesh->SetupAttachment(ReloadPartMesh);

	ReloadCollisionLocation = CreateDefaultSubobject<UBoxComponent>("Reload Collision Location");
	ReloadCollisionLocation->SetupAttachment(GetRootComponent());
	ReloadCollisionLocation->OnComponentBeginOverlap.AddDynamic(this, &AAutoLoadingGun::OnMagazineReloadOverlapEnter);

	SliderGrabCollisionLocation = CreateDefaultSubobject<UBoxComponent>("Slider Grab Collision Location");
	SliderGrabCollisionLocation->SetupAttachment(ReloadPartMesh);
	SliderGrabCollisionLocation->ComponentTags.Add("ALG_Slider");

	OffHandGrabLocationAndCollision = CreateDefaultSubobject<UBoxComponent>("OffHand Grab Location");
	OffHandGrabLocationAndCollision->SetupAttachment(GetRootComponent());
	OffHandGrabLocationAndCollision->ComponentTags.Add("ALG_OffHandGrab");
}							 

void AAutoLoadingGun::BeginPlay()
{
	Super::BeginPlay();

	DefaultTimeBetweenShots = 60 / RoundsPerMinute;
	CurrentShotDelay = DefaultTimeBetweenShots;

	if (GunFireModesAllowed.bAutomaticModeAllowed)
		eCurrentFireMode = eGunFireMode::EGFM_FullyAutomaticMode;
	else if (GunFireModesAllowed.bSemiAutomaticModeAllowed)
		eCurrentFireMode = eGunFireMode::EGFM_SemiAutomaticMode;
	else if (GunFireModesAllowed.bBurstModeAllowed)
		eCurrentFireMode = eGunFireMode::EGFM_BurstModeMode;	
}

void AAutoLoadingGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentShotDelay < DefaultTimeBetweenShots)
	{
		HandleWeaponShooting(DeltaTime);
	}
	else
	{
		if (!bUsesShellsInsteadOfMagazine)
		{
			HandleHoldingSlider(DeltaTime);
		}
		else
		{
			HandleShells();
		}
		
		switch (eCurrentFireMode)
		{
		case eGunFireMode::EGFM_Other:
			break;
		case eGunFireMode::EGFM_FullyAutomaticMode:
			bCanFireAgain = true;
			break;
		case eGunFireMode::EGFM_SemiAutomaticMode:
			if (!bTriggerPressedThisFrame) bCanFireAgain = true;
			break;
		case eGunFireMode::EGFM_BurstModeMode:
			if (!bTriggerPressedThisFrame && BurstShotsLeft <= 0) bCanFireAgain = true;
			break;
		default:
			break;
		}
	}

	if (bOffHandHoldingWeapon)
		HandleTwoHandedWeaponAiming();

	
	
	CustomGunTick(DeltaTime);

	bTriggerPressedThisFrame = false;
}

void AAutoLoadingGun::TriggerPressed()
{
	bTriggerPressedThisFrame = true;

	if (bCanFireAgain && bBulletInChamber && !bHoldingSlider)
	{
		//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "WEAPON FIRED!", true);
		//DrawDebugLine(GetWorld(), BarrelEnd->GetComponentLocation(), BarrelEnd->GetComponentLocation() + (BarrelEnd->GetForwardVector() * 100000), FColor::Red, false, 1.0f);
		CurrentShotDelay = 0;// DefaultTimeBetweenShots;
		bCanFireAgain = false;
		bBulletInChamber = false;

		if (UVRGameInstance* VRGI = Cast<UVRGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
		{
			if (VRGI->GetAudioManager())
			{
				VRGI->GetAudioManager()->PlayAudioClip(ShootingAudio, GetActorLocation());
			}
		}

		if (eCurrentFireMode == eGunFireMode::EGFM_BurstModeMode)
			BurstShotsLeft = GunFireModesAllowed.ShotsPerBurst;
	}
}

void AAutoLoadingGun::BottomButtonPressed()
{
	if (bQuickMagEject)
	{
		if (CurrentMagazine != NULL)
		{
			//CurrentMagazine->MainHandReleased();
			CurrentMagazine->DetachFromGun();
			CurrentMagazine = NULL;
		}
	}
}

void AAutoLoadingGun::TopButtonPressed()
{
	switch (eCurrentFireMode)
	{
	case eGunFireMode::EGFM_FullyAutomaticMode:
		if (GunFireModesAllowed.bSemiAutomaticModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_SemiAutomaticMode;
		else if(GunFireModesAllowed.bBurstModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_BurstModeMode;
		break;
	case eGunFireMode::EGFM_SemiAutomaticMode:
		if (GunFireModesAllowed.bBurstModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_BurstModeMode;
		else if (GunFireModesAllowed.bAutomaticModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_FullyAutomaticMode;
		break;
	case eGunFireMode::EGFM_BurstModeMode:
		if (GunFireModesAllowed.bAutomaticModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_FullyAutomaticMode;
		else if (GunFireModesAllowed.bSemiAutomaticModeAllowed)
			eCurrentFireMode = eGunFireMode::EGFM_SemiAutomaticMode;
		break;
	default:
		break;
	}
}

void AAutoLoadingGun::OffHandGrabbed(AVRPhysicsHand* Hand, const FName& PartNameGrabbed)
{	
	if (PartNameGrabbed == "ALG_Slider")
	{
		if (!Hand)
			return;

		bStuckInEmptyPos = false;
		bHoldingSlider = true;
		OffHand = Hand;
		//OffHand->SetActorLocation(GunSliderMesh->GetComponentLocation());
		OffHandStartingLocation = OffHand->GetMotionController()->GetComponentLocation();
		ActorLocationWhenGrabbed = GetActorLocation();

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Grabbed Gun Slider", true);

		/*Could do with improvements later
		* Used to work out the offset from where the hand grapped the slider
		* checks the distance a few times, while it only does it once, can be quite performance heavy
		*/
		FVector A = GetActorLocation();
		FVector B = A + (-ReloadPartMesh->GetForwardVector() * ReloadSliderOffset);
		FVector P = OffHand->GetMotionController()->GetComponentLocation();
		
		FVector Result = ExtraMaths::PointProjectionOnLine(A, B, P);
		
		float Dist = FVector::Distance(ReloadPartMesh->GetComponentLocation(), Result);
		DistanceGrabbedOnLine = Dist;
		
		float ResultToEndDist = FVector::Distance(B, Result);
		
		if (Dist < ResultToEndDist)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 500.0f, FColor::Blue, "True", true);
			bGrabbedFrontSide = true;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 500.0f, FColor::Blue, "False", true);
			bGrabbedFrontSide = false;
		}
		
	}
	else if (PartNameGrabbed == "ALG_OffHandGrab")
	{
		bOffHandHoldingWeapon = true;
		OffHand = Hand;
	}
}

void AAutoLoadingGun::OffHandReleased()
{
	OffHand = NULL;

	bHoldingSlider = false;
	bOffHandHoldingWeapon = false;
	OffHandStartingLocation = FVector(0, 0, 0);
	DistanceGrabbedOnLine = 0;

	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Blue, "Offhand Released!", true);
}

void AAutoLoadingGun::RemovedMagazineFromGun()
{
	if (bCanRemoveMagWithHands)
	{
		if (CurrentMagazine != NULL)
		{
			//CurrentMagazine->MainHandReleased();
			//CurrentMagazine->DetachFromGun();
			CurrentMagazine = NULL;
		}
	}
}

void AAutoLoadingGun::OnMagazineReloadOverlapEnter(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& HitResult)
{
	if (OtherActor != NULL && OtherComp != NULL)
	{
		if (OtherComp->ComponentHasTag("Magazine"))
		{
			AGunMagazine* ThisMag = Cast<AGunMagazine>(OtherActor);
			if (CurrentMagazine == NULL && ThisMag != NULL)
			{
				if (ThisMag->GetClass() == DefaultGunMagazine)
				{
					if (ThisMag->HoldingInHand() && (MainHand != NULL || OffHand != NULL))
					{
						if (ThisMag->AttachToGun(this, LoadedMagazineOffset))
						{
							CurrentMagazine = ThisMag;
						}
						//CurrentMagazine->AddActorLocalOffset(LoadedMagazineOffset * ItemBaseMesh->GetForwardVector());
					}
				}
			}
		}		
	}
}

void AAutoLoadingGun::HandleWeaponShooting(float DeltaTime)
{
	double SliderTime = 1000 * (60 / RoundsPerMinute);
	double MoveAmount = (SliderShootingOffset / SliderTime) * 2;

	if (CurrentShotDelay < DefaultTimeBetweenShots / 2)
	{
		CurrentRelativeLocation += (MoveAmount * 1000) * DeltaTime;

		if (CurrentRelativeLocation > SliderShootingOffset)
			CurrentRelativeLocation = SliderShootingOffset;
	}
	else
	{
		if (CurrentMagazine != NULL)
		{
			if (CurrentMagazine->GetAmmoLeft() > 0)
			{
				CurrentRelativeLocation -= (MoveAmount * 1000) * DeltaTime;

				if (CurrentRelativeLocation < 0)
					CurrentRelativeLocation = 0;
			}
			else
			{
				BurstShotsLeft = 0;
				bStuckInEmptyPos = true;

				if (UVRGameInstance* VRGI = Cast<UVRGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
				{
					if (VRGI->GetAudioManager())
					{
						VRGI->GetAudioManager()->PlayAudioClip(EmptyAudio, GetActorLocation());
					}
				}
			}
		}
		else
			bStuckInEmptyPos = true;
	}

	CurrentShotDelay += DeltaTime;

	if (CurrentMagazine != NULL && CurrentShotDelay >= DefaultTimeBetweenShots)
	{
		if (CurrentMagazine->GetAmmoLeft() > 0)
		{
			bBulletInChamber = true;
			CurrentMagazine->RemoveABullet();
			CurrentRelativeLocation = 0;

			if (BurstShotsLeft > 0)
			{
				BurstShotsLeft -= 1;
				CurrentShotDelay = 0;
			}
		}
	}

	ShootingPartMesh->SetWorldLocation(ItemBaseMesh->GetComponentLocation() + (-BarrelEnd->GetForwardVector() * CurrentRelativeLocation));

	if (bReloadPartMovesWhenShooting)
		ReloadPartMesh->SetWorldLocation(ItemBaseMesh->GetComponentLocation() + (-BarrelEnd->GetForwardVector() * CurrentRelativeLocation));
}

void AAutoLoadingGun::HandleHoldingSlider(float DeltaTime)
{
	if (bHoldingSlider)
	{
		if (OffHand != NULL && MainHand != NULL)
		{
			HandleSliderMovement();
		}
	}
	else
	{
		if (!bStuckInEmptyPos)
		{
			ReloadPartMesh->SetWorldLocation(GetActorLocation());
			ShootingPartMesh->SetWorldLocation(GetActorLocation());
		}
		else
		{
			ShootingPartMesh->SetWorldLocation(GetActorLocation() + (-GetActorForwardVector() * GunEmptySliderOffset));
			
			if(bReloadPartMovesWhenShooting)
				ReloadPartMesh->SetWorldLocation(GetActorLocation() + (-GetActorForwardVector() * GunEmptySliderOffset));
		}
	}
}

void AAutoLoadingGun::HandleSliderMovement()
{
	if (!OffHand)
		return;

	if (!OffHand->GetMotionController())
		return;

	FVector A = GetActorLocation();
	FVector B = A + (-ReloadPartMesh->GetForwardVector() * ReloadSliderOffset);
	FVector P = OffHand->GetMotionController()->GetComponentLocation();

	FVector Result = ExtraMaths::PointProjectionOnLine(A, B, P);

	float MoveOffset = DistanceGrabbedOnLine;
	
	float DistToMesh = FVector::Distance(A, ReloadPartMesh->GetComponentLocation());

	bGrabbedFrontSide ? Result += (-GetActorForwardVector() * MoveOffset)
		: Result += (GetActorForwardVector() * MoveOffset);

	float Dist = FVector::Distance(A, Result);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Distance: " + FString::SanitizeFloat(Dist), true);

	float EndToPointDist = FVector::Distance(B, Result);


	if (EndToPointDist > ReloadSliderOffset && Dist < EndToPointDist)
	{
		ReloadPartMesh->SetWorldLocation(GetActorLocation());
		ShootingPartMesh->SetWorldLocation(GetActorLocation());
	}
	else if (Dist < ReloadSliderOffset)
	{
		ReloadPartMesh->SetWorldLocation(Result);

		ShootingPartMesh->SetWorldLocation(Result);

		if (!bCanEjectBullet)
		{
			if (FVector::Distance(GetActorLocation(), Result) < ReloadSliderOffset - 0.5f)
				bCanEjectBullet = true;
		}
	}
	else
	{
		FVector NewLocation = GetActorLocation() +
			(-ReloadPartMesh->GetForwardVector() * ReloadSliderOffset);

		ReloadPartMesh->SetWorldLocation(NewLocation);

		ShootingPartMesh->SetWorldLocation(NewLocation);

		if (bBulletInChamber)
		{
			if (bCanEjectBullet)
			{
				bCanEjectBullet = false;
				bBulletInChamber = false;

				if (CurrentMagazine)
				{
					if (CurrentMagazine->GetAmmoLeft() > 0)
					{
						CurrentMagazine->RemoveABullet();
						bBulletInChamber = true;

						if (UVRGameInstance* VRGI = Cast<UVRGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
						{
							if (VRGI->GetAudioManager())
							{
								VRGI->GetAudioManager()->PlayAudioClip(SliderFullyBackAudio, GetActorLocation());
							}
						}
					}
				}
			}
		}
		else
		{
			if (CurrentMagazine)
			{
				float CurrentMagCap = CurrentMagazine->GetAmmoLeft();
				if (CurrentMagCap > 0)
				{
					CurrentMagazine->RemoveABullet();
					bBulletInChamber = true;
				}
			}
		}

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Gun Fully Back!", true);
	}
}

void AAutoLoadingGun::HandleShells()
{
	if (ShellsLeft > 0)
	{
		ShellsLeft -= 1;
		bBulletInChamber = true;

		CurrentRelativeLocation = 0;
		//GunSliderMesh->SetWorldLocation(GunSliderMesh->GetComponentLocation() + (BarrelEnd->GetForwardVector() * CurrentRelativeLocation));
	}
	else
	{
		CurrentRelativeLocation = 0;
		//GunSliderMesh->SetWorldLocation(GunSliderMesh->GetComponentLocation() + (BarrelEnd->GetForwardVector() * GunEmptySliderOffset));
	}
}

void AAutoLoadingGun::HandleTwoHandedWeaponAiming()
{
	if (!OffHand || !MainHand)
		return;

	if (!OffHand->GetMotionController() || !MainHand->GetMotionController())
		return;

	if (MainHand && OffHand)
	{
		if (bTwoHandedGun)
		{
			FVector AimDirection = OffHand->GetMotionController()->GetComponentLocation() - MainHand->GetMotionController()->GetComponentLocation();;
			AimDirection.Normalize();
			FRotator NewRotation = FRotationMatrix::MakeFromX(AimDirection).Rotator();
			SetActorRotation(NewRotation);
		}
	}
}
