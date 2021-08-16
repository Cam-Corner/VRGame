// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MPGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAMPGameModeBase, Log, All);

class APlayerController;

class AMPPlayerController;
/**
 * 
 */
UCLASS()
class VRGAME_API AMPGameModeBase : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMPGameModeBase();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void HandlePlayerJoined(APlayerController* NewPlayer);

private:
	TArray<AMPPlayerController*> PlayerControllers;
};
