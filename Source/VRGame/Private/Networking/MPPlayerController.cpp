// Fill out your copyright notice in the Description page of Project Settings.


#include "Networking/MPPlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/SpectatorPawn.h"
#include "Player/VRSpectatorPawn.h"

AMPPlayerController::AMPPlayerController()
{

}

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();


}

void AMPPlayerController::UnPossessPawn()
{
	if (GetLocalRole() >= ROLE_Authority)
	{
		if (!bSpectating)
		{
			CachedMyCharacter = GetPawn();
			UnPossess();

			CachedSpectatorPawn = GetWorld()->SpawnActor<AVRSpectatorPawn>();
			CachedSpectatorPawn->SetActorLocation(CachedMyCharacter->GetActorLocation());
			GEngine->AddOnScreenDebugMessage(42, 2.0f, FColor::Yellow, TEXT("UnPossessedPawn"));

			Possess(CachedSpectatorPawn);
		}
	}
}
