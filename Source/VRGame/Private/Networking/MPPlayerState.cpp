// Fill out your copyright notice in the Description page of Project Settings.


//#include "Networking/MPPlayerState.h"
#include "Networking/NetworkingHelpers.h"

AMPPlayerState::AMPPlayerState()
{

}

void AMPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerState, Username);
}