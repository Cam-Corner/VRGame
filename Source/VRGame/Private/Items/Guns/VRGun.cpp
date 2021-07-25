// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Guns/VRGun.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

// Sets default values
AVRGun::AVRGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	BarrelEnd = CreateDefaultSubobject<UArrowComponent>(TEXT("Barrel End"));
	BarrelEnd->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRGun::BeginPlay()
{
	Super::BeginPlay();
		
}

// Called every frame
void AVRGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}


void AVRGun::CustomGunTick(float DeltaTime)
{

}
