// Fill out your copyright notice in the Description page of Project Settings.


#include "TestingClass.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ATestingClass::ATestingClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateAbstractDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(Root);

	Goto = CreateAbstractDefaultSubobject<UStaticMeshComponent>(TEXT("Goto"));
	Goto->SetupAttachment(GetRootComponent());

	Moving = CreateAbstractDefaultSubobject<UStaticMeshComponent>(TEXT("Moving"));
	Moving->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ATestingClass::BeginPlay()
{
	Super::BeginPlay();
}
float OldForce = 0;
// Called every frame
void ATestingClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//F = -Kx
	//F force
	//K Stiffness
	//X Distance
	FVector LocA = Goto->GetComponentLocation();
	FVector LocB = Moving->GetComponentLocation();

	float Diff = FVector::Distance(LocA, LocB);
	float ForceAgainst = FVector(LocA - LocB).Size();
	float SpringForce = Spring.Update(Diff, ForceAgainst);
	FVector Dir = LocB - LocA;
	Dir.Normalize();

	Moving->SetPhysicsLinearVelocity(Dir * SpringForce);
}

