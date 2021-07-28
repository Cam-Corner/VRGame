// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HostJoinMenu.generated.h"

class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class VRGAME_API UUW_HostJoinMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UFUNCTION(BlueprintCallable)
		bool IsNewServerError(FString& GetError);

	UFUNCTION(BlueprintNativeEvent)
		void bp_CallNewError(const FString& GetError);

private:
	UFUNCTION()
	void JoinServer(FName IP);

	UFUNCTION()
	void JoinCameronsServer();

	UFUNCTION()
	void HostServer();

	bool bJoinServerTab = false;

	UPROPERTY(Meta = (BindWidget))
	UButton* Button_HostMatch;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* Text_HostMatch;

	UPROPERTY(Meta = (BindWidget))
	UButton* Button_JoinMatch;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* Text_JoinMatch;

	UPROPERTY(Meta = (BindWidget))
	UButton* Button_JoinCameron;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* Text_JoinCameron;

	float CheckErrorsTimer = 1;
};
