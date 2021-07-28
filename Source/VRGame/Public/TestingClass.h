// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/ExtraMaths.h"
#include "TestingClass.generated.h"

USTRUCT()
struct FLinearSpring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ForceMultiplier = 1.0f;

	float Update(float Difference, float ForceAgainst)
	{
		float K = ForceAgainst / Difference;
		float F = -K * Difference;
		return F * ForceMultiplier;
	}
};

/*USTRUCT()
struct FLinearSpring3D
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ForceMultiplier = 1.0f;

	FVector Update(FVector Difference, FVector ForceAgainst)
	{
		XSpring.ForceMultiplier = ForceMultiplier;
		YSpring.ForceMultiplier = ForceMultiplier;
		ZSpring.ForceMultiplier = ForceMultiplier;

		float X = XSpring.Update(Difference.X, ForceAgainst.X);
		float X = XSpring.Update(Difference.X, ForceAgainst.X);
		float X = XSpring.Update(Difference.X, ForceAgainst.X);
	}

private:
	FLinearSpring XSpring;
	FLinearSpring YSpring;
	FLinearSpring ZSpring;
};*/

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

	UPROPERTY(EditAnywhere, Category = "Spring")
	FLinearSpring Spring;
};
