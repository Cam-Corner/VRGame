// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem.h"
#include "Networking/NetworkingHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Networking/NetworkingHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AVRItem::AVRItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("Item Base Mesh");
	ItemBaseMesh->SetSimulatePhysics(true);
	RootComponent = ItemBaseMesh;

	OnCalcCustomPhysics.BindUObject(this, &AVRItem::CustomPhysics);
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

	if (ItemBaseMesh->GetBodyInstance())
	{
		ItemBaseMesh->GetBodyInstance()->AddCustomPhysics(OnCalcCustomPhysics);
	}

	if (!bItemHeald && MainHand)
		MainHand = NULL;
}

bool AVRItem::MainHandGrabbed(AVRPhysicsHand* Hand)
{
	if (bItemHeald)
		return false;

	MainHand = Hand;
	bItemHeald = true;

	return true;
}

void AVRItem::MainHandReleased()
{
	if(MainHand)
		MainHand->DropItemsInHand();

	bShouldDropNextFrame = false;
	bItemHeald = false;
	MainHand = NULL;
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

void AVRItem::OffHandGrabbed(AVRPhysicsHand* Hand, const FName& PartNameGrabbed)
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

void AVRItem::MoveItemToHand(float DeltaTime)
{
	if (!MainHand)
		return;

	FBodyInstance* BI = ItemBaseMesh->GetBodyInstance();

	FVector CurrentLoc = BI->GetUnrealWorldTransform().GetLocation();
	CurrentLoc += BI->GetUnrealWorldTransform().GetRotation().GetForwardVector() * -11;

	FVector F = FVector::ZeroVector;

	if (bUsePDController)
	{
		F = LocPD.GetForce(DeltaTime, CurrentLoc,
			MainHand->GetTrackingHandTransform().GetLocation());
	}
	else
	{
		F = LocSpring.Update(CurrentLoc, MainHand->GetTrackingHandTransform().GetLocation(),
			BI->GetUnrealWorldVelocity(), FVector::ZeroVector);
	}

	BI->AddForce(F * BI->GetBodyMass(), false);

	FQuat CQuat = BI->GetUnrealWorldTransform().GetRotation();
	FQuat DQuat = MainHand->GetTrackingHandTransform().GetRotation();
	FVector Vel = BI->GetUnrealWorldAngularVelocityInRadians();
	FVector IT = BI->GetBodyInertiaTensor();

	if (bTwoHanded && OffHand)
	{
		/*FVector ForwardV = BI->GetUnrealWorldTransform().GetRotation().GetForwardVector();
		FQuat ForwardQ = FQuat.m
		CQuat = ForwardQ;*/

		FVector Dir = OffHand->GetMotionController()->GetComponentLocation()
			- MainHand->GetMotionController()->GetComponentLocation();
		Dir.Normalize();

		FQuat Add = Dir.ToOrientationQuat();
		FQuat Diff = Add * DQuat.Inverse();
		DQuat = Diff * DQuat;

	}


	FVector T = FVector::ZeroVector;
		
	if (bUsePDController)
	{
		T = RotPD.GetTorque(DeltaTime, CQuat, DQuat, Vel, IT);
	}
	else
	{
		T = RotSpring.GetRequiredTorque(CQuat, DQuat, Vel, FVector::ZeroVector);
	}
		
	BI->AddTorqueInRadians(T * BI->GetBodyMass(), false);

	if (MainHand)
	{
		MainHand->SetActorLocation(GetMainHandGrip()->GetComponentLocation());
		MainHand->SetActorRotation(GetMainHandGrip()->GetComponentRotation());
	}
	
	if (OffHand)
	{
		OffHand->SetActorLocation(GetOffHandGrip()->GetComponentLocation());
		OffHand->SetActorRotation(GetOffHandGrip()->GetComponentRotation());
	}

}

UPrimitiveComponent* AVRItem::GetMainHandGrip_Implementation()
{
	return NULL;
}

UPrimitiveComponent* AVRItem::GetOffHandGrip_Implementation()
{
	return NULL;
}

void AVRItem::PhysicsTick_Implementation(float SubsetDeltaTime)
{
	
}

void AVRItem::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	MoveItemToHand(DeltaTime);
}

void AVRItem::Server_PickedupItem_Implementation(AVRPhysicsHand* HandToFollow)
{
	if (MainHand)
	{
		ClientDropWeapon(MainHand);
	}
	else
	{
		MainHand = HandToFollow;

		SetOwner(HandToFollow->GetOwner());
	}
}

void AVRItem::ClientDropWeapon_Implementation(AVRPhysicsHand* CurrentHandHoldingItem)
{
	MainHand = CurrentHandHoldingItem;
	MainHandReleased();
}

void AVRItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRItem, MainHand);
}