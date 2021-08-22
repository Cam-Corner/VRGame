// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRHandTesting.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"

// Sets default values
AVRHandTesting::AVRHandTesting()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	MotionController->SetupAttachment(RootComponent);

	NonPhysicsHand = CreateDefaultSubobject<UStaticMeshComponent>("NonPhysicsHand");
	NonPhysicsHand->SetupAttachment(MotionController);

	PhysicsHand = CreateDefaultSubobject<UStaticMeshComponent>("PhysicsHand");
}

// Called when the game starts or when spawned
void AVRHandTesting::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHand->SetWorldLocation(NonPhysicsHand->GetComponentLocation());
}

// Called every frame
void AVRHandTesting::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLoc = PhysicsHand->GetComponentLocation();
	FVector DesiredLoc = NonPhysicsHand->GetComponentLocation();
//	PhysicsHand->AddForce(MovementPID.Update(DeltaTime, CurrentLoc, DesiredLoc));

}

