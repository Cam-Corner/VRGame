// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AudioManager.generated.h"

class UAudioComponent;
class USoundCue;

UCLASS()
class VRGAME_API AAudioManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAudioManager();

	/*Play A sound clip*/
	UFUNCTION(BlueprintCallable)
	void PlayAudioClip(USoundCue* Sound, FVector Location);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	/*All the audio components that can be used in the game*/
	UPROPERTY(EditAnywhere)
	TArray<UAudioComponent*> AudioComponents;

	/*Fill up the audio component array with audio components*/
	void FillAudioComponentsArray(unsigned int Amount);

	/*The master audio that will be used to edit autio volume*/
	UPROPERTY(EditAnywhere)
	float MasterAudioVolume = 0.5f;
};
