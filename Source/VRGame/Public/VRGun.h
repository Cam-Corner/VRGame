// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRWeapon.h"
#include "VRGun.generated.h"


UCLASS()
class VRGAME_API AVRGun : public AVRWeapon
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRGun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	/* Called everyframe by the parent weapon class so they can have custom tick functions aswell as share the original tick */
	virtual void CustomGunTick(float DeltaTime);

/*=========
My Variables & Components
==========*/
protected:
	/* Gun Barrel End */
	UPROPERTY(VisibleAnywhere, Category = "Gun Barrel End")
	class UArrowComponent* BarrelEnd;


};