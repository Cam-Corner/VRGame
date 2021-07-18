// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VRGameInstance.generated.h"

class AAudioManager;

/**
 * 
 */
UCLASS()
class VRGAME_API UVRGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

public:
	UVRGameInstance();

	AAudioManager* GetAudioManager() { return AudioManager; }
	
	void SpawnAudioManager();
private:
	

	AAudioManager* AudioManager;
};
