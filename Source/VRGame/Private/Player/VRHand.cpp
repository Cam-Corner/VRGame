// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRHand.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Items/VRItem.h"
#include "Items/Guns/AutoLoadingGun.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"


// Sets default values
AVRHand::AVRHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = RootComp;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Controller"));
	MotionController->SetupAttachment(RootComp);
	MotionController->MotionSource = "Left";

	HandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hand Mesh"));
	HandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	HandMesh->SetupAttachment(MotionController);
	HandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HandMeshCol = CreateDefaultSubobject<USphereComponent>(TEXT("Hand Mesh Collision"));
	HandMeshCol->SetupAttachment(MotionController);
	HandMeshCol->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HandMeshCol->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	HandMeshCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	HandMeshCol->OnComponentBeginOverlap.AddDynamic(this, &AVRHand::HandGrabSphereOverlapBegin);
	HandMeshCol->OnComponentEndOverlap.AddDynamic(this, &AVRHand::HandGrabSphereOverlapEnd);

	WidgetIC = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetIC"));
	WidgetIC->InteractionDistance = 150.0f;
	WidgetIC->SetupAttachment(HandMesh);
	WidgetIC->bShowDebug = false;
	WidgetIC->DebugColor = FColor::Blue;
	WidgetIC->bEnableHitTesting = true;
}

// Called when the game starts or when spawned
void AVRHand::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForUIHits();

	if (ItemComponentInHand.PartGrabbedItem != NULL)
	{
		if (ItemComponentInHand.ItemPartGrabbed == "ALS_Slider")
		{
			HandMesh->SetWorldLocation(ItemComponentInHand.PartGrabbed->GetComponentLocation());
			//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Holding Weapon Slider!", true);
		}
	}
}

void AVRHand::CheckForUIHits()
{
	if(WidgetIC->IsOverFocusableWidget())
		WidgetIC->bShowDebug = true;
	else
		WidgetIC->bShowDebug = false;

}

void AVRHand::DropItemsInHand()
{
	if (ItemInHand != NULL)
	{
		ItemInHand = NULL;
	}
}

/*==========
Improve this later
============*/
void AVRHand::GripPressed(float Value)
{
	if (Value > 0.5f && !bBeingHeld)
	{
		bBeingHeld = true;
		bool bGrabbedItem = false;

		if (ItemInHand == NULL && OverlappedItems.Num() > 0)
		{
			if (OverlappedItems[0]->MainHandGrabbed(this))
			{
				ItemInHand = OverlappedItems[0];

				if (ItemInHand)
				{
					ItemInHand->SetActorLocation(MotionController->GetComponentLocation());
					ItemInHand->SetActorRotation(MotionController->GetComponentRotation());					
					ItemInHand->AttachToComponent(MotionController, FAttachmentTransformRules::KeepWorldTransform);							
					bGrabbedItem = true;
				}

			}
			//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Grabbed Gun Left", true);
		}
		
		if (ItemComponentInHand.PartGrabbedItem == NULL && OverlappedItemComponents.Num() > 0 && !bGrabbedItem)
		{

			/*if (OverlappedItemComponents[0].ItemPartGrabbed == "ALG_Slider")
			{
				ItemComponentInHand = OverlappedItemComponents[0];
				ItemComponentInHand.PartGrabbedItem->OffHandGrabbed(this, "Slider");
			}*/
			ItemComponentInHand = OverlappedItemComponents[0];
			ItemComponentInHand.PartGrabbedItem->OffHandGrabbed(this, ItemComponentInHand.ItemPartGrabbed);
		}
	}
	else
	{
		if (!bBeingHeld)
		{
			if (ItemInHand != NULL)
			{
				ItemInHand->MainHandReleased();
				ItemInHand = NULL;
			}

			if (ItemComponentInHand.PartGrabbedItem != NULL)
			{
				ItemComponentInHand.PartGrabbedItem->OffHandReleased();
				ItemComponentInHand.PartGrabbedItem = NULL;
				ItemComponentInHand.PartGrabbedItem = NULL;
				ItemComponentInHand.ItemPartGrabbed = "NotNamed";
			}

			HandMesh->SetWorldLocation(MotionController->GetComponentLocation());
		}



		if(Value < 0.5f)
			bBeingHeld = false;
	}
}

void AVRHand::TriggerPressed(float Value)
{
	if (Value > 0.05f)
	{
		if (ItemInHand != NULL)
		{
			ItemInHand->TriggerPressed();	
		}
		else
			UWInteractPressed();
	}
	else
	{
		UWInteractReleased();
	}
}

void AVRHand::BottomButtonPressed()
{
	if (ItemInHand != NULL)
	{
		ItemInHand->BottomButtonPressed();
	}
}

void AVRHand::TopButtonPressed()
{
	if (ItemInHand != NULL)
	{
		ItemInHand->TopButtonPressed();
	}
}

UFUNCTION()
void AVRHand::HandGrabSphereOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& HitResult)
{
	if (OtherActor != NULL)
	{
		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
		if (ThisItem != NULL)
		{
			AAutoLoadingGun* ThisALGun = Cast<AAutoLoadingGun>(OtherActor);
			if (ThisALGun != NULL)
			{
				if (ThisALGun->CanGrabItem())//HoldingInHand())
				{
					OverlappedItems.Add(ThisItem);
				}
				else
				{
					if (OtherComp != NULL)
					{
						if (OtherComp->ComponentTags.Num() > 0)
						{							
							//if (OtherComp->ComponentTags[0] == "ALG_Slider")
							{
								FItemComponent FIC;
								FIC.ItemPartGrabbed = OtherComp->ComponentTags[0];//"ALG_Slider";
								FIC.PartGrabbedItem = ThisALGun;
								FIC.PartGrabbed = OtherComp;
								OverlappedItemComponents.Add(FIC);
							}
						}
					}
				}
			}
			else
			{
				if (ThisItem->CanGrabItem())//HoldingInHand())
				{
					OverlappedItems.Add(ThisItem);
				}
			}
		}
	}
}

UFUNCTION()
void AVRHand::HandGrabSphereOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != NULL)
	{
		AVRItem* ThisALGun = Cast<AVRItem>(OtherActor);
		if (ThisALGun != NULL)
		{
			if (!ThisALGun->HoldingInHand())
			{
				OverlappedItems.Remove(ThisALGun);
			}
			else
			{
				if (OtherComp != NULL)
				{
					for(int i = 0; i < OverlappedItemComponents.Num(); i++)
					{
						if (OverlappedItemComponents[i].PartGrabbed == OtherComp)
						{
							OverlappedItemComponents.RemoveAt(i);
							break;
						}
					}
				}
			}
		}
	}
}

void AVRHand::UWInteractPressed()
{
	if (WidgetIC == NULL)
		return;

	WidgetIC->PressPointerKey(EKeys::LeftMouseButton);
}

void AVRHand::UWInteractReleased()
{
	if (WidgetIC == NULL)
		return;

	WidgetIC->ReleasePointerKey(EKeys::LeftMouseButton);
}
