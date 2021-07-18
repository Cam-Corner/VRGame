// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AAudioManager::AAudioManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AAudioManager::PlayAudioClip(USoundCue* Sound, FVector Location)
{
	if (AudioComponents.Num() <= 0 || !Sound)
		return;

	for (UAudioComponent* AC : AudioComponents)
	{
		if (AC)
		{
			if (!AC->IsPlaying())
			{
				AC->SetWorldLocation(Location);
				AC->Sound = Sound;
				AC->Play();
				break;
			}
		}
	}
}

// Called when the game starts or when spawned
void AAudioManager::BeginPlay()
{
	Super::BeginPlay();

	FillAudioComponentsArray(1000);
}

void AAudioManager::FillAudioComponentsArray(unsigned int Amount)
{
	for (int i = 0; i < (int)Amount; i++)
	{
		FString CompName = "AudioComp" + FString::FromInt(i);
		UAudioComponent* AC = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass(), FName(CompName));
		AudioComponents.Add(AC);
	}
}


