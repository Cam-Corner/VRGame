// Fill out your copyright notice in the Description page of Project Settings.


#include "Networking/NetworkingHelpers.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(LogAMPGameModeBase);

AMPGameModeBase::AMPGameModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOB(
		 TEXT("/Game/MyStuff/Blueprints/Player/bp_VRCharacter"));
	if (PlayerPawnOB.Succeeded())
		DefaultPawnClass = PlayerPawnOB.Class;

	PlayerControllerClass = AMPPlayerController::StaticClass();
	PlayerStateClass = AMPPlayerState::StaticClass();
	GameStateClass = AMPGameState::StaticClass();
}

void AMPGameModeBase::BeginPlay()
{
	Super::BeginPlay();

}

void AMPGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AMPPlayerController* PC = Cast<AMPPlayerController>(NewPlayer))
	{
		PlayerControllers.Add(PC);

		UE_LOG(LogAMPGameModeBase, Warning, TEXT("Client Connected!"));
	}
}

void AMPGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);


	if (AMPPlayerController* PC = Cast<AMPPlayerController>(NewPlayer))
	{
		PlayerControllers.Add(PC);

		UE_LOG(LogAMPGameModeBase, Warning, TEXT("Client Connected!"));
	}
}
