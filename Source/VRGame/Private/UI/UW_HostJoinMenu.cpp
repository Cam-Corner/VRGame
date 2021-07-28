// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_HostJoinMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "VRGameInstance.h"

#define LEVEL_GunRange TEXT("/Game/MyStuff/Levels/lvl_GunRangeV1");

#define GameMode_MP_Default TEXT("?listen?game=Default");

void UUW_HostJoinMenu::NativeConstruct()
{
	Button_HostMatch->OnPressed.AddDynamic(this, &UUW_HostJoinMenu::HostServer);
	Button_JoinCameron->OnPressed.AddDynamic(this, &UUW_HostJoinMenu::JoinCameronsServer);

	FString Error;
	if (IsNewServerError(Error))
	{
		bp_CallNewError(Error);
	}
}

void UUW_HostJoinMenu::JoinServer(FName IP)
{
	//UGameplayStatics::OpenLevel(GetWorld(), IP, true);
	FString Open = "open " + IP.ToString();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->ConsoleCommand(Open);
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

void UUW_HostJoinMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

}

bool UUW_HostJoinMenu::IsNewServerError(FString& GetError)
{
	if(UVRGameInstance* GI = Cast<UVRGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		return GI->IsNewServerError(GetError);
	}

	return false;
}

void UUW_HostJoinMenu::bp_CallNewError_Implementation(const FString& GetError)
{
}
