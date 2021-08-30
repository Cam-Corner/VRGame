#pragma once

#include "CoreMinimal.h"
#include "Utility/ExtraMaths.h"
#include "PIDControllers.generated.h"

/** Do Not Use This For Quaternions, Quaternions have their own unique PD Controller*/
template<typename T> struct FPDControllerT
{
public:
	FPDControllerT() {}

	T GetForce(float DeltaTime, T CurrentValue, T DesiredValue)
	{
		return InternalUpdate(DeltaTime, CurrentValue, DesiredValue);
	}

	void SetProportionalAndDerivative(float P, float D) { SetProportional(P); SetDerivative(D); }
	void SetProportional(float Value) { Proportional = Value; }
	void SetDerivative(float Value) { Derivative = Value; }
protected:
	T InternalUpdate(float DeltaTime, T CurrentValue, T DesiredValue)
	{
		float P = (6.f * Proportional) * (6.f * Proportional) * 0.25f;
		float D = 4.5f * Proportional * Derivative;

		T ThisError = DesiredValue - CurrentValue;
		T ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
		T Output = (P * ErrorPrior) + (ThisDerivative * D);

		ErrorPrior = ThisError;

		return Output;
	}

	/**How fast it takes to reach the target */	
	float Proportional = 1.0f;

	/* 
	* Value = 1: Critically Damped
	* Value < 1: Under Damped and will oscillate depending on how low the value is
	* Value > 1: Over Damped and will be slow to reach the target 
	*/
	float Derivative = 1.0f;

private:
	T ErrorPrior;
};

USTRUCT(BlueprintType)
struct FPDController
{
	GENERATED_BODY()
public:
	FPDController() {}

	float GetForce(float DeltaTime, float CurrentValue, float DesiredValue)
	{
		PDController.SetProportionalAndDerivative(Proportional, Derivative);
		return PDController.GetForce(DeltaTime, CurrentValue, DesiredValue);
	}

protected:
	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Proportional = 1.0f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Derivative = 1.0f;

private:
	FPDControllerT<float> PDController;
};

USTRUCT(BlueprintType)
struct FPDController3D
{
	GENERATED_BODY()
public:
	FPDController3D() {}

	FVector GetForce(float DeltaTime, FVector CurrentValue, FVector DesiredValue)
	{
		PDController.SetProportionalAndDerivative(Proportional, Derivative);
		return PDController.GetForce(DeltaTime, CurrentValue, DesiredValue);
	}

protected:
	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Proportional = 10.0f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Derivative = 1.0f;

private:
	FPDControllerT<FVector> PDController;
};

USTRUCT(BlueprintType)
struct FQuatPDController
{
	GENERATED_BODY()

public:
	FQuatPDController() {}

	/*
	* CQuat = Current Quaternion Value
	* DQuat = Desired Quaternion Value
	*/
	FVector GetTorque(float DeltaTime, FQuat CQuat, FQuat DQuat, FVector AVel, FVector InertiaTensor)
	{
		float P = Frequency;//(6.f * Frequency) * (6.f * Frequency) * 0.25f;
		float D = Dampening;//4.5f * Frequency * Dampening;

		FVector Axis = FVector::ZeroVector;
		float Angle = 0;
		FQuat Error = DQuat * CQuat.Inverse();

		if (Error.W < 0)
		{
			Error.X = -Error.X;
			Error.Y = -Error.Y;
			Error.Z = -Error.Z;
			Error.W = -Error.W;
		}

		Error.ToAxisAndAngle(Axis, Angle);
		Axis.Normalize();
		FVector::DegreesToRadians(Axis);
		FVector Value = P * Axis * Angle - D * AVel;
		/*FQuat RotInertia2World = InertiaTensor.ToOrientationQuat() * CQuat;
		Value = RotInertia2World.Inverse() * Value;
		Value *= InertiaTensor;
		Value = RotInertia2World * Value;*/
		return Value * ForceMultiplier;
	}

protected:
	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Frequency = 1.0f;

	/*
	* Value = 1: Critically Damped
	* Value < 1: Under Damped and will oscillate depending on how low the value is
	* Value > 1: Over Damped and will be slow to reach the target
	*/
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float Dampening = 1.0f;

	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float ForceMultiplier = 1.0f;
};
