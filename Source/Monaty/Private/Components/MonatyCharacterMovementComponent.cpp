// Copyright Conkis Studios, all rights reserved.

#include "Components/MonatyCharacterMovementComponent.h"

#include "Curves/CurveVector.h"

UMonatyCharacterMovementComponent::UMonatyCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMonatyCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                            const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		MaxWalkSpeed = NewMaxWalkSpeed;
		MaxWalkSpeedCrouched = NewMaxWalkSpeed;
	}
}

void UMonatyCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UMonatyCharacterMovementComponent::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UMonatyCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

void UMonatyCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UMonatyCharacterMovementComponent::Server_SetMaxWalkingSpeed_Implementation(const float UpdateMaxWalkSpeed)
{
	NewMaxWalkSpeed = UpdateMaxWalkSpeed;
}

float UMonatyCharacterMovementComponent::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped(FVector2f{LocWalkSpeed, LocRunSpeed}, FVector2f{1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped(FVector2f{0.0f, LocWalkSpeed}, FVector2f{0.0f, 1.0f}, Speed);
}

void UMonatyCharacterMovementComponent::SetMovementSettings(FPlayerMovementSettings NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
}

void UMonatyCharacterMovementComponent::SetMaxWalkingSpeed(float UpdateMaxWalkSpeed)
{
	if (UpdateMaxWalkSpeed != NewMaxWalkSpeed)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			NewMaxWalkSpeed = UpdateMaxWalkSpeed;
			Server_SetMaxWalkingSpeed(UpdateMaxWalkSpeed);
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}
