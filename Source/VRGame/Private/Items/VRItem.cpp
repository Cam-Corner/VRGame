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
}

// Called when the game starts or when spawned
void AVRItem::BeginPlay()
{
	Super::BeginPlay();
	
	//ItemBaseMesh->OnComponentHit.AddDynamic(this, &AVRItem::OnItemHit);
	//OnCalcCustomPhysics.BindUObject(this, &AVRItem::CustomPhysics);
	
}

// Called every frame
void AVRItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//MoveItemToHand(DeltaTime);

	/*if (ItemBaseMesh->GetBodyInstance())
	{
		ItemBaseMesh->GetBodyInstance()->AddCustomPhysics(OnCalcCustomPhysics);
	}*/

	if (!bItemHeald && MainHand)
		MainHand = NULL;
}

bool AVRItem::MainHandGrabbed(AVRPhysicsHand* Hand)
{
	if (bItemHeald)
		return false;

	MainHand = Hand;
	bItemHeald = true;
	//ItemBaseMesh->SetEnableGravity(false);
	//ItemBaseMesh->SetMassOverrideInKg(NAME_None, 0.001f, true);
	//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Weapon Grabbed", true);
	//ItemBaseMesh->SetEnableGravity(false);
	//ItemBaseMesh->SetSimulatePhysics(false);

	return true;
}

void AVRItem::MainHandReleased()
{
	if(MainHand)
		MainHand->DropItemsInHand();

	bShouldDropNextFrame = false;
	bItemHeald = false;
	//ItemBaseMesh->SetEnableGravity(true);
	MainHand = NULL;
	//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Weapon Released", true);
	//DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//ItemBaseMesh->SetMassOverrideInKg(NAME_None, 10, true);
	//ItemBaseMesh->SetSimulatePhysics(true);
}

void AVRItem::TriggerPressed()
{
	//if(GetLocalRole() >= ROLE_)
	//if(GetOwner() == UGameplayStatics::GetPlayerController())
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

	//Location
	/*FVector CurrentVel = ItemBaseMesh->GetBodyInstance()->GetUnrealWorldVelocity();
	FVector DesiredVel = MainHand->GetMotionController()->GetPhysicsLinearVelocity();
	FVector CurrentLoc = ItemBaseMesh->GetComponentLocation();
	FVector DesiredLoc = MainHand->GetMotionControllerLocation();

	FVector SpringForce = MovementSpring.Update(CurrentLoc, DesiredLoc, CurrentVel, DesiredVel);
	
	/*if(!bHitSomething)
		ItemBaseMesh->GetBodyInstance()->AddForce(SpringForce, false);
	else
		ItemBaseMesh->GetBodyInstance()->AddForce(FVector::ZeroVector, false, true);
	
	//MovementPID.SetPIDValue(Proportional, Integral, Derivative);
	ItemBaseMesh->GetBodyInstance()->AddForce(MovementPID.Update(DeltaTime, CurrentLoc, DesiredLoc), false);
	ItemBaseMesh->GetBodyInstance()->AddForce(SpringForce, false);

	bHitSomething = false;
	
	//Rotation
	FVector RCurrentVel = ItemBaseMesh->GetBodyInstance()->GetUnrealWorldAngularVelocityInRadians();
	RCurrentVel = FVector::RadiansToDegrees(RCurrentVel);
	FVector RDesiredVel = MainHand->GetMotionController()->GetPhysicsAngularVelocity();
	
	FVector CurrentRot = GetActorForwardVector();
	FVector DesiredRot = MainHand->GetMotionController()->GetForwardVector();
	FVector FinalTorque = RotationSpring.GetRequiredTorque(CurrentRot, DesiredRot, RCurrentVel, RDesiredVel);

	CurrentRot = GetActorRightVector();
	DesiredRot = MainHand->GetMotionController()->GetRightVector();
	FinalTorque += RotationSpring.GetRequiredTorque(CurrentRot, DesiredRot, RCurrentVel, RDesiredVel);

	CurrentRot = GetActorUpVector();
	DesiredRot = MainHand->GetMotionController()->GetUpVector();
	FinalTorque += RotationSpring.GetRequiredTorque(CurrentRot, DesiredRot, RCurrentVel, RDesiredVel);

	ItemBaseMesh->GetBodyInstance()->AddTorqueInRadians(FVector::DegreesToRadians(FinalTorque), false);*/


	/*SetActorLocation(MainHand->GetActorLocation());

	FQuat MQuat = MainHand->GetActorQuat();
	MQuat *= FQuat::MakeFromEuler(FVector(-90, 0, 0));
	SetActorRotation(MQuat);

	AddActorWorldOffset(GetActorForwardVector() * 4);
	AddActorWorldOffset(GetActorRightVector() * -1.5f);*/

	/*FVector HandF = MainHand->GetActorForwardVector();
	HandF.Normalize();
	FQuat HandQuat = HandF.ToOrientationQuat();
	FQuat Diff = HandQuat.Inverse() * MQuat;
	FQuat NewQuat = GetActorQuat();
	NewQuat = NewQuat * Diff;
	SetActorRotation(NewQuat);*/

}

UPrimitiveComponent* AVRItem::GetGripConstraint_Implementation()
{
	return NULL;
}

UPrimitiveComponent* AVRItem::GetOffHandGripConstraint_Implementation()
{
	return NULL;
}

void AVRItem::OnItemHit(class UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	bHitSomething = true;
	LastHitResult = Hit;
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