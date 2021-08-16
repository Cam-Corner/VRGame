// Fill out your copyright notice in the Description page of Project Settings.


#include "RigidBodyTest.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/VRHandAnimInstance.h"



// Sets default values
ARigidBodyTest::ARigidBodyTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HandSK = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand SK"));
	RootComponent = HandSK;

}

// Called when the game starts or when spawned
void ARigidBodyTest::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARigidBodyTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FollowActor)
		return;


}

