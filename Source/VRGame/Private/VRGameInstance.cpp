// Fill out your copyright notice in the Description page of Project Settings.


#include "VRGameInstance.h"
#include "Audio/AudioManager.h"

UVRGameInstance::UVRGameInstance()
{
	
}

void UVRGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, 
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	SetNewServerError(ErrorString);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue,
		"Server Error: " + ErrorString, true);
}

void UVRGameInstance::DoOnce()
{
	if (GameInstanceDoOnce)
	{
		GetEngine()->OnNetworkFailure().AddUObject(this, &UVRGameInstance::HandleNetworkFailure);	
		GameInstanceDoOnce = false;
	}
}

void UVRGameInstance::SpawnAudioManager()
{
	if (GameInstanceDoOnce)
	{
		GameInstanceDoOnce = false;
	}

	if (GetWorld())
	{
		AudioManager = GetWorld()->SpawnActor<AAudioManager>();
	}
}
