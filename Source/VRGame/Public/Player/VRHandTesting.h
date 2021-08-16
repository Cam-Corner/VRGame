// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PhysicsSprings.h"
#include "VRHandTesting.generated.h"

class UStaticMeshComponent;
class UMotionControllerComponent;
class USceneComponent;

UCLASS()
class VRGAME_API AVRHandTesting : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRHandTesting();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere)
		UMotionControllerComponent* MotionController;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* NonPhysicsHand;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* PhysicsHand;

	UPROPERTY(EditAnywhere)
		FPIDController3D MovementPID;
};
