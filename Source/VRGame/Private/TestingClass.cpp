// Fill out your copyright notice in the Description page of Project Settings.


#include "TestingClass.h"
#include "Utility/PhysicsSprings.h"
//#include "Utility/ExtraMaths.h"


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

	OnCalculateCustomPhysics.BindUObject(this, &ATestingClass::CustomPhysics);
}

// Called when the game starts or when spawned
void ATestingClass::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ATestingClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Moving->GetBodyInstance())
	{
		Moving->GetBodyInstance()->AddCustomPhysics(OnCalculateCustomPhysics);
	}
}

void ATestingClass::PhysicsTick(float SubstepDeltaTime, FBodyInstance* BodyInstance)
{
	FVector F = LocPD.GetForce(SubstepDeltaTime, BodyInstance->GetUnrealWorldTransform().GetLocation(), 
		Goto->GetComponentLocation());

	BodyInstance->AddForce(BodyInstance->GetBodyMass() * F, false);

	FQuat CQuat = Moving->GetComponentQuat();
	FQuat DQuat = Goto->GetComponentQuat();
	FVector Vel = BodyInstance->GetUnrealWorldAngularVelocityInRadians();
	FVector IT = BodyInstance->GetBodyInertiaTensor();

	FVector T = RotPD.GetTorque(SubstepDeltaTime, CQuat, DQuat, Vel, IT);
	BodyInstance->AddTorqueInRadians(T, false);// / SubstepDeltaTime);
}

void ATestingClass::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	PhysicsTick(DeltaTime, BodyInstance);
}

