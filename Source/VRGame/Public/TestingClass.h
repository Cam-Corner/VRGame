// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PhysicsSprings.h"
#include "Utility/PIDControllers.h"
#include "Kismet/KismetMathLibrary.h"
#include "TestingClass.generated.h"

UCLASS()
class VRGAME_API ATestingClass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestingClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Testing")
		class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Testing")
		class UStaticMeshComponent* Goto;

	UPROPERTY(VisibleAnywhere, Category = "Testing")
		class UStaticMeshComponent* Moving;

	UPROPERTY(EditAnywhere, Category = "PD Tuning")
		FPDController3D LocPD;

	UPROPERTY(EditAnywhere, Category = "PD Tuning")
		FQuatPDController RotPD;

	void PhysicsTick(float SubstepDeltaTime, FBodyInstance* BodyInstance);
	FCalculateCustomPhysics OnCalculateCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);
};

