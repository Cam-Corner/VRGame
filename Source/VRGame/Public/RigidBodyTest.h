// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PhysicsSprings.h"
#include "RigidBodyTest.generated.h"



class USkeletalMeshComponent;

UCLASS()
class VRGAME_API ARigidBodyTest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARigidBodyTest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
		USkeletalMeshComponent* HandSK;

	UPROPERTY(EditAnywhere, Category = "ActorFollow")
		AActor* FollowActor;

	UPROPERTY(EditAnywhere, Category = "PID Controllers")
		FQuatPDController QuatPD;

};
