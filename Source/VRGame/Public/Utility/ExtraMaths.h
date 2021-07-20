// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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
