// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/ExtraMaths.h"
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

	UPROPERTY(EditAnywhere, Category = "Location PID")
		float LocForce = 100;

	UPROPERTY(EditAnywhere, Category = "Location PID")
		float LocP = 0;

	UPROPERTY(EditAnywhere, Category = "Location PID")
		float LocI = 0;

	UPROPERTY(EditAnywhere, Category = "Location PID")
		float LocD = 0;

	FPIDController3D LocPID;

	UPROPERTY(EditAnywhere, Category = "Rotation PID")
		float RotForce = 100;

	UPROPERTY(EditAnywhere, Category = "Rotation PID")
		float RotP = 0;

	UPROPERTY(EditAnywhere, Category = "Rotation PID")
		float RotI = 0;

	UPROPERTY(EditAnywhere, Category = "Rotation PID")
		float RotD = 0;

	FPIDController3D RotPID;

	FPIDController RotDPID;

	FVector LastCross = FVector(0, 0, 0);

};
