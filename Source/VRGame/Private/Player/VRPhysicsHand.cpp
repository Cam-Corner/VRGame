// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRPhysicsHand.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "Player/VRTrackingHands.h"
#include "UObject/ConstructorHelpers.h"
#include "Items/VRItem.h"
#include "Items/Guns/AutoLoadingGun.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SphereComponent.h"
#include "Networking/NetworkingHelpers.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Player/VRHandAnimInstance.h"

// Sets default values
AVRPhysicsHand::AVRPhysicsHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//SetReplicates(true);
	bReplicates = true;

	HandSK = CreateDefaultSubobject<USkeletalMeshComponent>("HandSkeletalMesh");
	HandSK->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HandSK->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	HandSK->SetSimulatePhysics(true);
	HandSK->SetEnableGravity(false);
	HandSK->SetMassOverrideInKg(NAME_None, 100);
	RootComponent = HandSK;

	HandGrabSphere = CreateDefaultSubobject<USphereComponent>("HandGrabSphere");
	HandGrabSphere->SetupAttachment(HandSK);
	HandGrabSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HandGrabSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	HandGrabSphere->SetCollisionProfileName("VRHand");
	HandGrabSphere->SetGenerateOverlapEvents(true);
	HandGrabSphere->SetVisibility(false);
	HandGrabSphere->bHiddenInGame = true;
	HandGrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRPhysicsHand::HandGrabSphereOverlapBegin);
	HandGrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRPhysicsHand::HandGrabSphereOverlapEnd);

	PhysicsConst = CreateDefaultSubobject<UPhysicsConstraintComponent>("PhysicsConst");
	PhysicsConst->SetupAttachment(HandSK);
	//PhysicsConst->SetDisableCollision(true);
	PhysicsConst->ConstraintActor1 = this;
	PhysicsConst->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	PhysicsConst->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	PhysicsConst->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> HandSKMesh
	(TEXT("/Game/VirtualReality/Mannequin/Character/Mesh/MannequinHand_Right.MannequinHand_Right"));
	if (HandSKMesh.Succeeded())
	{
		HandSK->SetSkeletalMesh(HandSKMesh.Object);
	}

	OnCalcCustomPhysics.BindUObject(this, &AVRPhysicsHand::CustomPhysics);
}

void AVRPhysicsHand::SetIsLeftHand()
{
	HandSK->SetWorldScale3D(FVector(1, 1, -1));
}

void AVRPhysicsHand::SetTrackingHands(AVRTrackingHands* Hands)
{
	TrackingHands = Hands;

	if (TrackingHands)
		return;

	FVector NewPos = TrackingHands->GetHandSkeletalMesh()->GetComponentLocation();
	NewPos.Z += 50;
	NewPos += TrackingHands->GetHandSkeletalMesh()->GetForwardVector() * 5;
	//SetActorLocation(NewPos, false, NULL, ETeleportType::ResetPhysics);

}

void AVRPhysicsHand::GripPressed(float Value)
{
	if (Value > 0.5f && !bBeingHeld)
	{
		bBeingHeld = true;
		bool bGrabbedItem = false;

		if (GripPose)
		{
			if (UVRHandAnimInstance* AnimI = Cast<UVRHandAnimInstance>(HandSK->GetAnimInstance()))
			{
				AnimI->bHandClosed = true;
			}

			bInGripPose = true;
		}

		if (ItemInHand == NULL)
		{
			if (OverlappedItems.Num() > 0)
			{
				if (OverlappedItems[0]->MainHandGrabbed(this))
				{
					ItemInHand = OverlappedItems[0];

					if (ItemInHand)
					{				
						if (ItemInHand->GetMainHandGrip())
						{
							DisableCollision();
							SetActorLocation(ItemInHand->GetMainHandGrip()->GetComponentLocation());
							SetActorRotation(ItemInHand->GetMainHandGrip()->GetComponentRotation());
							bGrabbedItem = true;
						}
					}

				}
			}			
		}

		if (ItemComponentInHand.PartGrabbedItem == NULL && OverlappedItemComponents.Num() > 0 && !bGrabbedItem)
		{

			ItemComponentInHand = OverlappedItemComponents[0];
			ItemComponentInHand.PartGrabbedItem->OffHandGrabbed(this, ItemComponentInHand.ItemPartGrabbed);
			DisableCollision();

			if (ItemComponentInHand.PartGrabbedItem->GetOffHandGrip())
			{
				SetActorLocation(ItemComponentInHand.PartGrabbedItem->GetMainHandGrip()->GetComponentLocation());
				SetActorRotation(ItemComponentInHand.PartGrabbedItem->GetMainHandGrip()->GetComponentRotation());
			}
		}
	}
	else
	{
		if (!bBeingHeld)
		{
			if (bInGripPose)
			{
				//if (IdlePose)
					//HandSK->PlayAnimation(IdlePose, true);

				if (UVRHandAnimInstance* AnimI = Cast<UVRHandAnimInstance>(HandSK->GetAnimInstance()))
				{
					AnimI->bHandClosed = false;
				}

				bInGripPose = false;
			}

			if (ItemInHand != NULL)
			{
				ItemInHand->MainHandReleased();
				ItemInHand = NULL;
				EnableCollision();
				//RotPD.ForceMultiplier = OldPIDForce;
					 
			}

			if (ItemComponentInHand.PartGrabbedItem != NULL)
			{
				ItemComponentInHand.PartGrabbedItem->OffHandReleased();
				ItemComponentInHand.PartGrabbedItem = NULL;
				ItemComponentInHand.PartGrabbed = NULL;
				ItemComponentInHand.ItemPartGrabbed = "NotNamed";
				EnableCollision();
			}

			PhysicsConst->BreakConstraint();

			/*LocPID.SetPIDValue(EmptyLocPIDSettings);
			RotPD.SetPIDValue(EmptyRotPIDSettings);*/
		}

		if (Value < 0.5f)
			bBeingHeld = false;
	}
}

void AVRPhysicsHand::TriggerPressed(float Value)
{
	if (Value > 0.05f)
	{
		if (ItemInHand != NULL)
		{
			ItemInHand->TriggerPressed();
		}
	}
}

void AVRPhysicsHand::BottomButtonPressed()
{
	if (ItemInHand != NULL)
	{
		ItemInHand->BottomButtonPressed();
	}
}

void AVRPhysicsHand::TopButtonPressed()
{
	if (ItemInHand != NULL)
	{
		ItemInHand->TopButtonPressed();
	}
}

void AVRPhysicsHand::DropItemsInHand()
{
	if (ItemInHand)
	{
		ItemInHand = NULL;
		//PhysicsConst->ConstraintActor2 = NULL;
	}
}

void AVRPhysicsHand::NonPhysicsHandLocation()
{
	if (!TrackingHands)
		return;

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetActorLocation(TrackingHands->GetMotionController()->GetComponentLocation());
		SetActorRotation(TrackingHands->GetHandSkeletalMesh()->GetComponentRotation());
		
		Server_SendLocAndRot(GetActorLocation(), GetActorRotation());	
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		//SetActorLocation(PlayerHandLoc);
		//SetActorRotation(PlayerHandRot);
	}
}

// Called when the game starts or when spawned
void AVRPhysicsHand::BeginPlay()
{
	Super::BeginPlay();
	HandSK->SetWorldScale3D(FVector(1, 1, 1));
	HandSK->SetSimulatePhysics(false);
}

void AVRPhysicsHand::HandGrabSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& HitResult)
{
	if (OtherActor != NULL)
	{
		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
		if (ThisItem != NULL)
		{
			AAutoLoadingGun* ThisALGun = Cast<AAutoLoadingGun>(OtherActor);
			if (ThisALGun != NULL)
			{
				if (ThisALGun->CanGrabItem())//HoldingInHand())
				{
					OverlappedItems.Add(ThisItem);
				}
				else
				{
					if (OtherComp != NULL)
					{
						if (OtherComp->ComponentTags.Num() > 0)
						{
							//if (OtherComp->ComponentTags[0] == "ALG_Slider")
							{
								FItemComponent FIC;
								FIC.ItemPartGrabbed = OtherComp->ComponentTags[0];//"ALG_Slider";
								FIC.PartGrabbedItem = ThisALGun;
								FIC.PartGrabbed = OtherComp;
								OverlappedItemComponents.Add(FIC);
							}
						}
					}
				}
			}
			else
			{
				if (ThisItem->CanGrabItem())//HoldingInHand())
				{
					OverlappedItems.Add(ThisItem);
				}
			}
		}
	}
}

void AVRPhysicsHand::HandGrabSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != NULL)
	{
		AVRItem* ThisALGun = Cast<AVRItem>(OtherActor);
		if (ThisALGun != NULL)
		{
			if (!ThisALGun->HoldingInHand())
			{
				OverlappedItems.Remove(ThisALGun);
			}
			else
			{
				if (OtherComp != NULL)
				{
					for (int i = 0; i < OverlappedItemComponents.Num(); i++)
					{
						if (OverlappedItemComponents[i].PartGrabbed == OtherComp)
						{
							OverlappedItemComponents.RemoveAt(i);
							break;
						}
					}
				}
			}
		}
	}
}

void AVRPhysicsHand::EnableCollision()
{
	HandSK->SetSimulatePhysics(true);
	HandSK->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AVRPhysicsHand::DisableCollision()
{
	HandSK->SetSimulatePhysics(false);
	HandSK->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVRPhysicsHand::PhysicsMoveCollisionToHand(float DeltaTime)
{
	if (!TrackingHands)
		return;

	FBodyInstance* BI = HandSK->GetBodyInstance();

	FVector F = LocPD.GetForce(DeltaTime, HandSK->GetComponentLocation(),
		TrackingHands->GetHandSkeletalMesh()->GetComponentLocation());
	BI->AddForce(F * BI->GetBodyMass(), false);

	FQuat CQuat = HandSK->GetComponentQuat();
	FQuat DQuat = TrackingHands->GetHandSkeletalMesh()->GetComponentQuat();
	FVector Vel = BI->GetUnrealWorldAngularVelocityInRadians();
	FVector IT = BI->GetBodyInertiaTensor();

	FVector T = RotPD.GetTorque(DeltaTime, CQuat, DQuat, Vel, IT);
	BI->AddTorqueInRadians(T, false);
}

void AVRPhysicsHand::MovePhysicsItemToHand(float DeltaTime)
{
	if (!TrackingHands || !ItemInHand)
		return;

	
}

// Called every frame
void AVRPhysicsHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bDoOnceOnTick && TrackingHands)
	{
		FVector NewPos = TrackingHands->GetHandSkeletalMesh()->GetComponentLocation();
		NewPos.Z += 50;
		SetActorLocation(NewPos, false, NULL, ETeleportType::TeleportPhysics);

		bDoOnceOnTick = true;
	}

	if (HandSK->GetBodyInstance() && HandSK->IsSimulatingPhysics())
	{
		HandSK->GetBodyInstance()->AddCustomPhysics(OnCalcCustomPhysics);
	}
	 
}

void AVRPhysicsHand::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	PhysicsMoveCollisionToHand(DeltaTime);
}

UMotionControllerComponent* AVRPhysicsHand::GetMotionController()
{
	if (TrackingHands)
		return TrackingHands->GetMotionController();

	return NULL;
}

const FTransform AVRPhysicsHand::GetTrackingHandTransform()
{
	if (TrackingHands)
		return TrackingHands->GetMotionController()->GetComponentTransform(); 

	return FTransform();
}

void AVRPhysicsHand::Server_SendLocAndRot_Implementation(FVector Loc, FRotator Rot)
{
	PlayerHandLoc = Loc;
	PlayerHandRot = Rot;
}

void AVRPhysicsHand::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRPhysicsHand, PlayerHandLoc);
	DOREPLIFETIME(AVRPhysicsHand, PlayerHandRot);
	DOREPLIFETIME(AVRPhysicsHand, TrackingHands);
}
