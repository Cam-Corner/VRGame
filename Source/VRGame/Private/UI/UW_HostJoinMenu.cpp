// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_HostJoinMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

#define LEVEL_GunRange TEXT("/Game/MyStuff/Levels/lvl_GunRangeV1");

#define GameMode_MP_Default TEXT("?listen?game=Default");

void UUW_HostJoinMenu::NativeConstruct()
{
	Button_HostMatch->OnPressed.AddDynamic(this, &UUW_HostJoinMenu::HostServer);
	Button_JoinCameron->OnPressed.AddDynamic(this, &UUW_HostJoinMenu::JoinCameronsServer);
}

void UUW_HostJoinMenu::JoinServer(FName IP)
{
	UGameplayStatics::OpenLevel(GetWorld(), IP, true);
}

void UUW_HostJoinMenu::HostServer()
{
	FName Map = LEVEL_GunRange;
	FString Mode = GameMode_MP_Default;
	UGameplayStatics::OpenLevel(GetWorld(), Map, true, Mode);
	//UEngine::Browse()
	//GetWorld()->ServerTravel(MapMode);
}

void UUW_HostJoinMenu::JoinCameronsServer()
{
	//92.168.0.1
	JoinServer("86.7.212.96");
}
