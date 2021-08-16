// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "VRSpectatorPawn.generated.h"

class UCameraComponent;

/**
 * 
 */
UCLASS()
class VRGAME_API AVRSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
public:
	AVRSpectatorPawn();

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UCameraComponent* VRCamera;

	UFUNCTION()
		void ForwardMovement(float Value);

	UFUNCTION()
		void RightMovement(float Value);

	UFUNCTION()
		void XRotation(float Value);

	UFUNCTION()
		void YRotation(float Value);

	void HandleMovement(float DeltaTime);

	void HandleRotation(float DeltaTime);

	float MovementSpeed = 650;

	float Sensitivity = 100;

	FVector MovementDir = FVector::ZeroVector;

	FVector RotationAxis = FVector::ZeroVector;

	FVector ConsumeMovementDir();

	FVector ConsumeRotation();

};
