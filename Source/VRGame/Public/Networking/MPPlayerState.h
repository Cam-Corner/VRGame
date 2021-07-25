// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPPlayerState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAMPPlayerState, Log, All);

/**
 * 
 */
UCLASS()
class VRGAME_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AMPPlayerState();

	/* This function only works and should only be called on the server */
	//void SetUsername(FString NewUsername) { Username = NewUsername; }

private:
	UPROPERTY(Replicated)
	FString Username = "BLANK!";
};
