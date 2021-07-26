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
	LocPID.SetPIDValue(LocP, LocI, LocD);
}

// Called every frame
void ATestingClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LocPID.SetPIDValue(LocP, LocI, LocD);
	FVector LocPIDUpdate = LocPID.Update(DeltaTime, Moving->GetComponentLocation(), Goto->GetComponentLocation());
	
	if (LocPIDUpdate.IsNearlyZero())
		LocPIDUpdate.Set(0, 0, 0);

	//Moving->AddForce(LocPIDUpdate * LocForce);
	RotDPID.SetPIDValue(RotP, RotI, RotD);
	FVector TargetDir = Moving->GetComponentLocation() - Goto->GetComponentLocation();
	TargetDir.Normalize();
	FVector RotDir = UKismetMathLibrary::FindLookAtRotation(Moving->GetForwardVector(), TargetDir).Vector();

	float XAngleError = FMath::FindDeltaAngle(Moving->GetComponentRotation().Euler().X,
		Goto->GetComponentRotation().Euler().X);
	float Torque = RotDPID.Update(DeltaTime, Moving->GetComponentRotation().Euler().X,
		Goto->GetComponentRotation().Euler().X);
	
	//Moving->AddTorque(FVector(0, 1, 0));
}

