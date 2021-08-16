// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRSpectatorPawn.h"
#include "Camera/CameraComponent.h"

AVRSpectatorPawn::AVRSpectatorPawn()
{
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RootComponent = VRCamera;

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
}


void AVRSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	/*=======
	Keyboard Input
	=======*/
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRSpectatorPawn::ForwardMovement);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRSpectatorPawn::RightMovement);

	/*=======
	Mouse Input
	=======*/
	PlayerInputComponent->BindAxis("XRotation", this, &AVRSpectatorPawn::XRotation);
	PlayerInputComponent->BindAxis("YRotation", this, &AVRSpectatorPawn::YRotation);
}

void AVRSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleMovement(DeltaTime);
	HandleRotation(DeltaTime);
}

void AVRSpectatorPawn::HandleMovement(float DeltaTime)
{
	FVector Dir = ConsumeMovementDir();

	AddActorWorldOffset(Dir * MovementSpeed * DeltaTime);
}

void AVRSpectatorPawn::HandleRotation(float DeltaTime)
{
	FVector Axis = ConsumeRotation();

	AddControllerYawInput(Axis.X * Sensitivity * DeltaTime);


	AddControllerPitchInput(Axis.Y * -Sensitivity * DeltaTime);
}

void AVRSpectatorPawn::ForwardMovement(float Value)
{
	MovementDir += GetActorForwardVector() * Value;
}

void AVRSpectatorPawn::RightMovement(float Value)
{
	MovementDir += GetActorRightVector() * Value;
}

void AVRSpectatorPawn::XRotation(float Value)
{
	RotationAxis.X = Value;
}

void AVRSpectatorPawn::YRotation(float Value)
{
	RotationAxis.Y = Value;
}

FVector AVRSpectatorPawn::ConsumeMovementDir()
{
	MovementDir.Normalize();
	FVector Dir = MovementDir;
	MovementDir = FVector::ZeroVector;
	return Dir;
}

FVector AVRSpectatorPawn::ConsumeRotation()
{
	FVector Axis = RotationAxis;
	RotationAxis = FVector::ZeroVector;
	return Axis;
}
