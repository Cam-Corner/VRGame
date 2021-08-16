// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PhysicsSprings.h"
#include "Kismet/KismetMathLibrary.h"
#include "TestingClass.generated.h"

USTRUCT()
struct FQuatPIDController

{
	GENERATED_BODY()

public:
	FQuatPIDController() {}
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	void SetPID()
	{
		XPID.SetPIDValue(ForceMultiplier, Proportional, Integral, Derivative);
		YPID.SetPIDValue(ForceMultiplier, Proportional, Integral, Derivative);
		ZPID.SetPIDValue(ForceMultiplier, Proportional, Integral, Derivative);
		WPID.SetPIDValue(ForceMultiplier, Proportional, Integral, Derivative);
	}

	FVector Update(float DeltaTime, FQuat CurrentQuat, FQuat DesiredQuat)
	{
		SetPID();

		FQuat A = CurrentQuat;
		FQuat B = DesiredQuat;
		A.Normalize();
		B.Normalize();
		FQuat Diff = B * A.Inverse();
		Diff.Normalize();
		Diff = Diff.Inverse();
		
		float Angle = CurrentQuat.AngularDistance(B);
		Angle = FMath::RadiansToDegrees(Angle);

		float Force = XPID.Update(DeltaTime, Angle, 0);

		return Diff.Euler() * Force * ForceMultiplier;
	}



protected:
	UPROPERTY(EditAnywhere)
		float ForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere)
		float Proportional = 0.05f;

	UPROPERTY(EditAnywhere)
		float Integral = 0.05f;

	UPROPERTY(EditAnywhere)
		float Derivative = 0.05f;

private:
	FPIDController XPID;
	FPIDController YPID;
	FPIDController ZPID;
	FPIDController WPID;
};

USTRUCT()
struct FOrientationPIDController
{
	GENERATED_BODY()

public:
	FOrientationPIDController() {}
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	FOrientationPIDController(float FM, float P, float I, float D)
	{
		SetPIDValue(FM, P, I, D);
	}

	FVector Update(float DeltaTime, FVector CurrentDir, FVector DesiredDir)
	{
		float ThisError = ExtraMaths::GetAngleOfTwoVectors(CurrentDir, DesiredDir); //ExtraMaths::GetAngleOfTwoVectors(CurrentUnitDir, DesiredUnitDir);
		float ThisIntegral = IntegralPrior + ThisError * DeltaTime;
		float ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
		float Output = (Proportional * ErrorPrior) + (ThisIntegral * Integral) + (ThisDerivative * Derivative);

		ErrorPrior = ThisError;
		IntegralPrior = ThisIntegral;

		FVector AngleOfRot = FVector::CrossProduct(CurrentDir, DesiredDir);

		return  Output * ForceMultiplier * AngleOfRot;
	}

	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	void SetPIDValue(float FM, float P, float I, float D)
	{
		ForceMultiplier = FM;
		Proportional = P;
		Integral = I;
		Derivative = D;
	}

protected:
	UPROPERTY(EditAnywhere)
		float ForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere)
		float Proportional = 0.05f;

	UPROPERTY(EditAnywhere)
		float Integral = 0.05f;

	UPROPERTY(EditAnywhere)
		float Derivative = 0.05f;

private:
	float ErrorPrior = 0;
	float IntegralPrior = 0;
};

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

	UPROPERTY(EditAnywhere, Category = "PID")
		FQuatPIDController QuatPID;

	UPROPERTY(EditAnywhere, Category = "PID")
		FPIDController ForwardVPID;

	UPROPERTY(EditAnywhere, Category = "PID")
		FPIDController RightVPID;

	UPROPERTY(EditAnywhere, Category = "PID")
		FPIDController UpVPID;

	UPROPERTY(EditAnywhere, Category = "PID")
		FTorsionalSpring RotSpring;

	UPROPERTY(EditAnywhere, Category = "PID")
		FOrientationPIDController XRotPID;
	UPROPERTY(EditAnywhere, Category = "PID")
		FOrientationPIDController YRotPID;
	UPROPERTY(EditAnywhere, Category = "PID")
		FOrientationPIDController ZRotPID;
};

