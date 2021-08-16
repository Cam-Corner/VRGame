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
	
	/*FVector CurrentRot = Moving->GetComponentQuat().Euler();
	FVector DesiredRot = Goto->GetComponentQuat().Euler();

	FVector XRot = XRotPID.Update(DeltaTime, CurrentRot.X, DesiredRot.X, -FVector::ForwardVector);
	FVector YRot = YRotPID.Update(DeltaTime, CurrentRot.Y, DesiredRot.Y, -FVector::RightVector);
	FVector ZRot = ZRotPID.Update(DeltaTime, CurrentRot.Z, DesiredRot.Z, FVector::UpVector);*/

	//Moving->AddTorque(XRot + YRot + ZRot);

	//FQuat CurrentRot = Moving->GetComponentQuat();
	//FQuat DesiredRot = Goto->GetComponentQuat();
	/*FQuat Diff = DesiredRot * CurrentRot.Inverse();
	FQuat NewQuat = Diff * CurrentRot;*/

	//Moving->SetWorldRotation(NewQuat);
	//FVector T = QuatPID.Update(DeltaTime, CurrentRot, DesiredRot);
	//Moving->AddTorque(T);


	/*FVector T = RotSpring.GetRequiredTorque(Moving->GetForwardVector(), Goto->GetForwardVector(),
		Moving->GetPhysicsAngularVelocity(), Goto->GetPhysicsAngularVelocity());
	Moving->AddTorque(T);*/

	/*FQuat CurrentRot = Moving->GetCompoen
	FQuat DesiredRot = Goto->GetRelativeRotation().Quaternion();
	FQuat Diff = CurrentRot * DesiredRot.Inverse();
	FVector ToEuler = Diff.Euler();
	FVector T = FVector(FMath::FindDeltaAngleDegrees(0, ToEuler.X),
		FMath::FindDeltaAngleDegrees(0, ToEuler.Y), FMath::FindDeltaAngleDegrees(0, ToEuler.Z));
	
	Moving->SetPhysicsAngularVelocity(T);*/

	FVector Current = Moving->GetForwardVector();
	FVector Desired = Goto->GetForwardVector();
	FVector T = XRotPID.Update(DeltaTime, Current, Desired);

	Current = Moving->GetRightVector();
	Desired = Goto->GetRightVector();
	T += YRotPID.Update(DeltaTime, Current, Desired);

	Current = Moving->GetUpVector();
	Desired = Goto->GetUpVector();
	T += ZRotPID.Update(DeltaTime, Current, Desired);

	Moving->SetPhysicsAngularVelocityInDegrees(T);
}

