// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "VRGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "VRCharacterComponent.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*===========================
	Initialize Root Component
	=============================*/
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	/*===========================
	Initialize VR Camera Components
	=============================*/
	VRCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRCameraRoot"));
	VRCameraRoot->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->AttachToComponent(VRCameraRoot, FAttachmentTransformRules::KeepWorldTransform);

	/*ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	ForwardDirection->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
	ForwardDirection->bHiddenInGame = false;
	ForwardDirection->AddLocalOffset(FVector(0, 0, 5));*/

	/*===========================
	Initialize left and right Motion Controllers
	=============================*/
	//LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	//LeftMotionController->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
	//LeftMotionController->MotionSource = "Left";

	//RightMotionController	= CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	//RightMotionController->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
	//RightMotionController->MotionSource = "Right";

	//LeftHandMesh 	= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandMesh"));
	//LeftHandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	//LeftHandMesh->AttachToComponent(LeftMotionController, FAttachmentTransformRules::KeepWorldTransform);
	//LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//LeftHandMeshCol = CreateDefaultSubobject<USphereComponent>(TEXT("LeftHandMeshCollision"));
	//LeftHandMeshCol->AttachToComponent(LeftMotionController, FAttachmentTransformRules::KeepWorldTransform);
	//LeftHandMeshCol->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//LeftHandMeshCol->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	//LeftHandMeshCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//LeftHandMeshCol->OnComponentBeginOverlap.AddDynamic(this, &AVRCharacter::LeftHandGrabSphereOverlapBegin);
	//LeftHandMeshCol->OnComponentEndOverlap.AddDynamic(this, &AVRCharacter::LeftHandGrabSphereOverlapEnd);

	//RightHandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandMesh"));
	//RightHandMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	//RightHandMesh->AttachToComponent(RightMotionController, FAttachmentTransformRules::KeepWorldTransform);
	//RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//RightHandMeshCol = CreateDefaultSubobject<USphereComponent>(TEXT("RightHandMeshCollision"));
	//RightHandMeshCol->AttachToComponent(RightMotionController, FAttachmentTransformRules::KeepWorldTransform);
	//RightHandMeshCol->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//RightHandMeshCol->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	//RightHandMeshCol->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//RightHandMeshCol->OnComponentBeginOverlap.AddDynamic(this, &AVRCharacter::RightHandGrabSphereOverlapBegin);
	//RightHandMeshCol->OnComponentEndOverlap.AddDynamic(this, &AVRCharacter::RightHandGrabSphereOverlapEnd);


	/*===========================
	Initialize Body Collision and setup Settings
	=============================*/
	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
	BodyCollision->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
	BodyCollision->AddWorldOffset(FVector(0, 0, BodyCollision->GetScaledCapsuleHalfHeight()));
	BodyCollision->SetSimulatePhysics(true);
	BodyCollision->SetEnableGravity(true);
	BodyCollision->SetVisibility(true);
	BodyCollision->SetCollisionProfileName("BlockAll");

	VRCharacterComponent = CreateDefaultSubobject<UVRCharacterComponent>(TEXT("VR Character Component"));
	//AddOwnedComponent(VRCharacterComponent);
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	//BodyCollision->bHiddenInGame = true;

	SpawnHands();

	if (UVRGameInstance* VRGI = Cast<UVRGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		VRGI->SpawnAudioManager();
	}

	if (VRCharacterComponent)
	{
		VRCharacterComponent->SetComponentOwner(this);
		VRCharacterComponent->SetVRCharacterCamera(VRCamera, VRCameraRoot);
		VRCharacterComponent->SetVRCharacterCapsule(BodyCollision);
	}
}

void AVRCharacter::SpawnHands()
{
	if (BP_DefaultHand != NULL)
	{
		//setup rules
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;

		FAttachmentTransformRules HandRule = FAttachmentTransformRules::SnapToTargetIncludingScale;
		HandRule.LocationRule = EAttachmentRule::SnapToTarget;
		HandRule.RotationRule = EAttachmentRule::SnapToTarget;
		HandRule.ScaleRule = EAttachmentRule::KeepWorld;

		//spawn left hand
		LeftHand = GetWorld()->SpawnActor<AVRHand>(BP_DefaultHand, FVector(0, 0, 0), FRotator(0, 0, 0), SpawnInfo);
		if (LeftHand != NULL)
		{
			LeftHand->SetMotionSource("Left");
			LeftHand->AttachToComponent(VRCameraRoot, HandRule);
		}

		//spawn right hand
		RightHand = GetWorld()->SpawnActor<AVRHand>(BP_DefaultHand, FVector(0, 0, 0), FRotator(0, 0, 0), SpawnInfo);
		if (RightHand != NULL)
		{
			RightHand->SetMotionSource("Right");
			RightHand->AttachToComponent(VRCameraRoot, HandRule);
		}
	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*Set the forward rotation to the Yaw of the camera
	FRotator NewRotation = ForwardDirection->GetComponentRotation();
	NewRotation.Yaw = VRCamera->GetComponentRotation().Yaw;
	ForwardDirection->SetWorldRotation(NewRotation);*/

	/*Call needed functions*/
	/*SmoothRotation(DeltaTime);
	ApplyMovement();
	ScaleCollisionWithPlayer();*/

	if (VRCharacterComponent)
	{
		VRCharacterComponent->SetXYMovementDirection(MovementThumbstick);
	}

}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/*=====================
	Thumbstick Input Settings
	=======================*/
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRCharacter::ForwardMovement);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRCharacter::RightMovement);
	PlayerInputComponent->BindAxis("XRotation", this, &AVRCharacter::XRotation);


	/*=====================
	Grip & Trigger Input Settings
	=======================*/
	PlayerInputComponent->BindAxis("LeftGrip", this, &AVRCharacter::LeftGripPressed);
	PlayerInputComponent->BindAxis("RightGrip", this, &AVRCharacter::RightGripPressed);
	PlayerInputComponent->BindAxis("LeftTrigger", this, &AVRCharacter::LeftTriggerPressed);
	PlayerInputComponent->BindAxis("RightTrigger", this, &AVRCharacter::RightTriggerPressed);

	/*=====================
	Controller Buttons
	=======================*/
	PlayerInputComponent->BindAction("LeftBottomButton", EInputEvent::IE_Pressed, this, &AVRCharacter::LeftBottomButtonPressed);
	PlayerInputComponent->BindAction("LeftTopButton", EInputEvent::IE_Pressed, this, &AVRCharacter::LeftTopButtonPressed);
	PlayerInputComponent->BindAction("RightBottomButton", EInputEvent::IE_Pressed, this, &AVRCharacter::RightBottomButtonPressed);
	PlayerInputComponent->BindAction("RightTopButton", EInputEvent::IE_Pressed, this, &AVRCharacter::RightTopButtonPressed);

}

void AVRCharacter::ForwardMovement(float Value)
{
	MovementThumbstick.Y = Value;
}

void AVRCharacter::RightMovement(float Value)
{
	MovementThumbstick.X = Value;
}

void AVRCharacter::ApplyMovement()
{
	//set a deadzone for the thumbsticks
	/*float ThumbstickDeadZone = 0.25f;

	FVector FinalMovementDirection{ 0, 0, 0 };
	float GravityMovement{ 0 };
	FVector2D CurrentMovementStickValues{ 0, 0 };


	/*Apply Forward Movement
	if (m_MovementThumbstick.Y > ThumbstickDeadZone || m_MovementThumbstick.Y < -ThumbstickDeadZone)
	{
		FinalMovementDirection += ForwardDirection->GetForwardVector() * m_MovementThumbstick.Y;
		CurrentMovementStickValues.Y = m_MovementThumbstick.Y;
	}

	/*Apply Right Movement
	if (m_MovementThumbstick.X > ThumbstickDeadZone || m_MovementThumbstick.X < -ThumbstickDeadZone)
	{
		FinalMovementDirection += ForwardDirection->GetRightVector() * m_MovementThumbstick.X;
		CurrentMovementStickValues.X = m_MovementThumbstick.X;
	}

	/*Check if grounded and if not then apply gravity
	{
		FHitResult FloorHit;
		FVector StartLocation = BodyCollision->GetComponentLocation();
		float SphereRadius = BodyCollision->GetScaledCapsuleRadius() - 2;
		FVector EndLocation = StartLocation - FVector(0, 0, ((BodyCollision->GetScaledCapsuleHalfHeight() - SphereRadius) + 1));
		FCollisionShape SphereTrace;
		FCollisionQueryParams SphereParams;
		SphereParams.AddIgnoredActor(this);

		SphereTrace.SetSphere(SphereRadius);

		bool bHitFloor = GetWorld()->SweepSingleByChannel(FloorHit, StartLocation, EndLocation,
														  BodyCollision->GetComponentQuat(), 
														  ECollisionChannel::ECC_Visibility,
														  SphereTrace, SphereParams);

		if(!bHitFloor)
			GravityMovement = GravitySpeed;
	}


	/*Add Final Force
	FVector FinalVelocity = FinalMovementDirection * MovementSpeed;
	FinalVelocity.Z -= GravityMovement;
	BodyCollision->SetPhysicsLinearVelocity(FinalVelocity);

	/*If the player is moving with locomotion then move the player with the body collision
	if (CurrentMovementStickValues != FVector2D(0, 0))
	{
		/*Set root location as caps location - half height
		FVector NewLocation = BodyCollision->GetComponentLocation();
		NewLocation.Z -= BodyCollision->GetScaledCapsuleHalfHeight();

		FVector DistanceCapCam = VRCamera->GetComponentLocation() - BodyCollision->GetComponentLocation();
		DistanceCapCam.Z = 0;

		NewLocation = GetActorLocation() - DistanceCapCam;

		SetActorLocation(NewLocation); //this is overriding rotation because it is constantly being recenter to capsule 
	}

	/*Set actor Z to Body Collision Z
	float BodyColZ = BodyCollision->GetComponentLocation().Z - BodyCollision->GetScaledCapsuleHalfHeight();
	float ZOffset = BodyColZ - GetActorLocation().Z;
	AddActorWorldOffset(FVector(0, 0, ZOffset));*/
}

void AVRCharacter::PIDController(const float DeltaTime, const float CurrentValue, const float DesiredValue, float &ErrorPrior, float &IntegralPrior)
{
	float ThisError = DesiredValue - CurrentValue;
	float ThisIntegral = IntegralPrior + ThisError * DeltaTime;
	float ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
	float Output = (Proportional * ErrorPrior) + (ThisIntegral * Integral) + (ThisDerivative * Derivative);

	ErrorPrior = ThisError;
	IntegralPrior = ThisIntegral;
}

void AVRCharacter::XRotation(float Value)
{
	if (VRCharacterComponent)
	{
		VRCharacterComponent->SetXRotationValue(Value);
	}
}

//
//UFUNCTION() void AVRCharacter::LeftHandGrabSphereOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
//	const FHitResult& HitResult)
//{
//	if (OtherActor != NULL)
//	{
//		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
//		if (ThisItem != NULL)
//		{
//			LeftOverlappedItems.Add(ThisItem);
//		}
//	}
//}
//
//UFUNCTION() void AVRCharacter::LeftHandGrabSphereOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (OtherActor != NULL)
//	{
//		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
//		if (ThisItem != NULL)
//		{
//			LeftOverlappedItems.Remove(ThisItem);
//		}
//	}
//}
//
//UFUNCTION() void AVRCharacter::RightHandGrabSphereOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
//	const FHitResult& HitResult)
//{	
//	if (OtherActor != NULL)
//	{
//		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
//		if (ThisItem != NULL)
//		{
//			RightOverlappedItems.Add(ThisItem);
//		}
//	}
//}
//
//UFUNCTION() void AVRCharacter::RightHandGrabSphereOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (OtherActor != NULL)
//	{
//		AVRItem* ThisItem = Cast<AVRItem>(OtherActor);
//		if (ThisItem != NULL)
//		{
//			RightOverlappedItems.Remove(ThisItem);
//		}
//	}
//}

void AVRCharacter::LeftGripPressed(float Value)
{
	//if (Value > 0.5f)
	//{
	//	if (LeftHandItem == NULL && LeftOverlappedItems.Num() > 0)
	//	{
	//		if (LeftOverlappedItems[0]->Grabbed())
	//		{
	//			LeftHandItem = LeftOverlappedItems[0];
	//			LeftHandItem->SetActorLocation(LeftMotionController->GetComponentLocation());
	//			LeftHandItem->SetActorRotation(LeftMotionController->GetComponentRotation());
	//			LeftOverlappedItems[0]->AttachToComponent(LeftMotionController, FAttachmentTransformRules::KeepWorldTransform);
	//		}
	//		//GEngine->AddOnScreenDebugMessage(-1, INFINITY, FColor::Yellow, "Grabbed Gun Left", true);
	//	}
	//}
	//else
	//{
	//	if (LeftHandItem != NULL)
	//	{			
	//		LeftHandItem->Dropped();
	//		LeftHandItem = NULL;
	//	}
	//}

	if (LeftHand != NULL)
	{
		LeftHand->GripPressed(Value);
	}
}

void AVRCharacter::RightGripPressed(float Value)
{
	if (RightHand != NULL)
	{
		RightHand->GripPressed(Value);
	}
}

void AVRCharacter::LeftTriggerPressed(float Value)
{
	if (LeftHand != NULL)
	{
		LeftHand->TriggerPressed(Value);
	}
}

void AVRCharacter::RightTriggerPressed(float Value)
{
	if (RightHand != NULL)
	{
		RightHand->TriggerPressed(Value);
	}
}

void AVRCharacter::LeftBottomButtonPressed()
{
	if (LeftHand != NULL)
	{
		LeftHand->BottomButtonPressed();
	}
}

void AVRCharacter::LeftTopButtonPressed()
{
	if (LeftHand != NULL)
	{
		LeftHand->TopButtonPressed();
	}
}

void AVRCharacter::RightBottomButtonPressed()
{
	if (RightHand != NULL)
	{
		RightHand->BottomButtonPressed();
	}
}

void AVRCharacter::RightTopButtonPressed()
{
	if (RightHand != NULL)
	{
		RightHand->TopButtonPressed();
	}
}