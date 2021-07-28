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

	
	void SetNewServerError(FString Error)
	{
		bNewServerError = true;
		ServerError = Error;
	}

	UFUNCTION(BlueprintCallable)
	bool IsNewServerError(FString& GetError)
	{
		if (!bNewServerError)
			return false;

		GetError = ServerError;
		bNewServerError = false;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	void DoOnce();

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
		ENetworkFailure::Type FailureType, const FString& ErrorString);

private:
	

	AAudioManager* AudioManager;

	bool bNewServerError = false;
	FString ServerError;

	bool GameInstanceDoOnce = true;
};
