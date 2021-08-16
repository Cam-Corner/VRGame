// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VRHandAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class VRGAME_API UVRHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	void SetForce(FVector Force) { ExternalForce = Force; }

	UPROPERTY(BlueprintReadOnly, Category = "HandPos")
		bool bHandClosed = false;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Forces")
		FVector ExternalForce = FVector::ZeroVector;

	
};
