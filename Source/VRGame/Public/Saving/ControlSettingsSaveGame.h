// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ControlSettingsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class VRGAME_API UControlSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	/* Wether the play is using smooth turning or not, if false they are using snap turning */
	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool bUsingSmoothTurning = false;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	float SmoothTurningSensitivity = 15.0f;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	float SnapTurningAmount = 45.0f;
};
