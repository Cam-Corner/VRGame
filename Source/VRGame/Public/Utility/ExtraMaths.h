// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
static class VRGAME_API ExtraMaths
{
public:

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
};
