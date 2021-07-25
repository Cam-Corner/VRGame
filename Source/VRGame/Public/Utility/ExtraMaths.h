// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//USTRUCT(Blueprintable)
struct FPIDController
{
	//GENERATED_BODY()

public:
	FPIDController() {}
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	FPIDController(float P, float I, float D)
	{
		SetPIDValue(P, I, D);
	}

	float Update(float DeltaTime, float CurrentValue, float DesiredValue)
	{
		float ThisError = DesiredValue - CurrentValue;
		float ThisIntegral = IntegralPrior + ThisError * DeltaTime;
		float ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
		float Output = (Proportional * ErrorPrior) + (ThisIntegral * Integral) + (ThisDerivative * Derivative);

		ErrorPrior = ThisError;
		IntegralPrior = ThisIntegral;

		return Output;
	}


	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	void SetPIDValue(float P, float I, float D)
	{
		Proportional = P;
		Integral = I;
		Derivative = D;
	}

protected:
	float Proportional = 0.05f;
	float Integral = 0.05f;
	float Derivative = 0.05f;

private:
	float ErrorPrior = 0;
	float IntegralPrior = 0;
};

//USTRUCT(Blueprintable)
struct FPIDController3D
{
	//GENERATED_BODY()

public:
	FPIDController3D() {}
	
	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	FPIDController3D(float P, float I, float D)
	{
		SetPIDValue(P, I, D);
	}

	/**
	* P = Proportional
	* I = Integral
	* D = Derivative
	*/
	void SetPIDValue(float P, float I, float D)
	{
		XPID.SetPIDValue(P, I, D);
		YPID.SetPIDValue(P, I, D);
		ZPID.SetPIDValue(P, I, D);
	}

	FVector Update(float DeltaTime, FVector CurrentValue, FVector DesiredValue)
	{
		FVector Result = FVector(0, 0, 0);

		Result.X = XPID.Update(DeltaTime, CurrentValue.X, DesiredValue.X);
		Result.Y = YPID.Update(DeltaTime, CurrentValue.Y, DesiredValue.Y);
		Result.Z = ZPID.Update(DeltaTime, CurrentValue.Z, DesiredValue.Z);

		return Result;
	}

private:
	float ErrorPrior = 0;
	float IntegralPrior = 0;

	FPIDController XPID;
	FPIDController YPID;
	FPIDController ZPID;
};

/**
 * 
 */
static class VRGAME_API ExtraMaths
{
public:

	/* returns the closest point on the line of VecA-VecB */
	static FVector PointProjectionOnLine(FVector VecA, FVector VecB, FVector Point)
	{
		FVector A = VecA;
		FVector B = VecB;
		FVector P = Point;

		FVector AP = P - A;
		FVector AB = B - A;
		FVector Result = A + FVector::DotProduct(AP, AB) / FVector::DotProduct(AB, AB) * AB;

		return Result;
	}

	/* returns the angle of the 2 vectors in degrees */
	static float GetAngleOfTwoVectors(const FVector& VecA, const FVector& VecB)
	{
		float MagA = VecA.Size();
		float MagB = VecB.Size();

		float DotProduct = FVector::DotProduct(VecA, VecB);

		float Angle = acos(DotProduct / (MagA * MagB));

		return FMath::RadiansToDegrees(Angle);
	}
};
