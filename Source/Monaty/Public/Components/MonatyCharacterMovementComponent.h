// Copyright Conkis Studios, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Character/MonatyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "MonatyCharacterMovementComponent.generated.h"

/**
 * Authoritative networked Character Movement
 */
UCLASS()
class MONATY_API UMonatyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Movement Settings Variables
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;

	UPROPERTY()
	float NewMaxWalkSpeed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Movement System")
	FPlayerMovementSettings CurrentMovementSettings;

	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FPlayerMovementSettings NewMovementSettings);

	// Set Max Walking Speed (Called from the owning client)
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMaxWalkingSpeed(float UpdateMaxWalkSpeed);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetMaxWalkingSpeed(float UpdateMaxWalkSpeed);
};
