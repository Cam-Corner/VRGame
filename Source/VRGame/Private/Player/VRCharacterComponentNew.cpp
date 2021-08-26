// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRCharacterComponentNew.h"
#include "Networking/NetworkingHelpers.h"

// Sets default values for this component's properties
UVRCharacterComponentNew::UVRCharacterComponentNew()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVRCharacterComponentNew::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UVRCharacterComponentNew::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UVRCharacterComponentNew::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVRCharacterComponentNew, SyncedHMDLocation);
}
