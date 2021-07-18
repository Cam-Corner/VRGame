// Fill out your copyright notice in the Description page of Project Settings.


#include "VRGameInstance.h"
#include "AudioManager.h"

UVRGameInstance::UVRGameInstance()
{
	
}

void UVRGameInstance::SpawnAudioManager()
{
	if (GetWorld())
	{
		AudioManager = GetWorld()->SpawnActor<AAudioManager>();
	}
}
