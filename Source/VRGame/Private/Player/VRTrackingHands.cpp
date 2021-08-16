// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRTrackingHands.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "Player/VRPhysicsHand.h"
#include "UObject/ConstructorHelpers.h"
#include "Networking/NetworkingHelpers.h"

// Sets default values
AVRTrackingHands::AVRTrackingHands()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	RootComponent = RootComp;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	MotionController->SetupAttachment(RootComp);

	HandSK = CreateDefaultSubobject<USkeletalMeshComponent>("HandSkeletalMesh");
	HandSK->SetupAttachment(MotionController);
	HandSK->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HandSK->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HandSK->AddLocalRotation(FRotator(-20, 0, 90));
	HandSK->AddLocalOffset(FVector(-10, 0, 0));

	ConstructorHelpers::FObjectFinder<USkeletalMesh> HandSKMesh
			(TEXT("/Game/VirtualReality/Mannequin/Character/Mesh/MannequinHand_Right.MannequinHand_Right"));
	if (HandSKMesh.Succeeded())
	{
		HandSK->SetSkeletalMesh(HandSKMesh.Object);
	}

	ConstructorHelpers::FObjectFinder<UMaterialInstance> HandMat
			(TEXT("/Game/MyStuff/Materials/m_TrackingHands.m_TrackingHands"));
	if (HandMat.Succeeded())
	{
		HandSK->SetMaterial(0, HandMat.Object);
	}
}

void AVRTrackingHands::SetTrackingHand(FName Hand)
{
	if (!MotionController)
		return;

	MotionController->MotionSource = Hand;

	if (Hand == "Left")
	{
		HandSK->SetWorldScale3D(FVector(1, 1, -1));

		if(PhysicsHand)
			PhysicsHand->SetIsLeftHand();
	}
}

void AVRTrackingHands::GripPressed(float Value)
{
	if (!PhysicsHand)
		return;

	PhysicsHand->GripPressed(Value);
}

void AVRTrackingHands::TriggerPressed(float Value)
{
	if (!PhysicsHand)
		return;

	PhysicsHand->TriggerPressed(Value);
}

void AVRTrackingHands::BottomButtonPressed()
{
	if (!PhysicsHand)
		return;

	PhysicsHand->BottomButtonPressed();
}

void AVRTrackingHands::TopButtonPressed()
{
	if (!PhysicsHand)
		return;

	PhysicsHand->TopButtonPressed();
}

bool AVRTrackingHands::SpawnPhysicsHands(AActor* NewOwner)
{
	//setup rules
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;

	FAttachmentTransformRules HandRule = FAttachmentTransformRules::SnapToTargetIncludingScale;
	HandRule.LocationRule = EAttachmentRule::SnapToTarget;
	HandRule.RotationRule = EAttachmentRule::SnapToTarget;
	HandRule.ScaleRule = EAttachmentRule::KeepWorld;

	//spawn left hand
	PhysicsHand = GetWorld()->SpawnActor<AVRPhysicsHand>(PhysicsHandsToSpawn,
		HandSK->GetComponentLocation(), HandSK->GetComponentRotation(), SpawnInfo);

	if (PhysicsHand)
	{
		PhysicsHand->SetTrackingHands(this);
		PhysicsHand->SetOwner(NewOwner);
		
		return true;
	}

	return false;
}

// Called when the game starts or when spawned
void AVRTrackingHands::BeginPlay()
{
	Super::BeginPlay();
	OldRoot = RootComp->GetComponentLocation();
	HandSK->bHiddenInGame = bHideHands;
	
}

// Called every frame
void AVRTrackingHands::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MotionController->IsTracked() && bCheckTracking)
	{
		if (PhysicsHand)
		{
			PhysicsHand->SetActorLocation(GetActorLocation());
		}
		
		bCheckTracking = false;
	}
	else if (!MotionController->IsTracked())
	{
		bCheckTracking = true;
	}
}

void AVRTrackingHands::Server_LocAndRot_Implementation(FVector Loc, FRotator Rot)
{
	HandLoc = Loc;
	HandRot = Rot;
}

void AVRTrackingHands::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRTrackingHands, PhysicsHand);
	DOREPLIFETIME(AVRTrackingHands, PhysicsHand);
	DOREPLIFETIME(AVRTrackingHands, PhysicsHand);
}

