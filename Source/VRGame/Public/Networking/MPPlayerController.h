// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogAMPPlayerController, Log, All);

class APawn;

/**
 * 
 */
UCLASS()
class VRGAME_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMPPlayerController();

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void UnPossessPawn();
private:

	APawn* CachedMyCharacter;
	APawn* CachedSpectatorPawn;

	bool bSpectating = false;
};
