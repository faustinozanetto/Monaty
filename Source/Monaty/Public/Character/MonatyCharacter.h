// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/PlaceablesComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "MonatyCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerGaitState : uint8
{
	None,
	Walking,
	Sprinting
};

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
	None,
	Grounded,
	InAir,
};

UENUM(BlueprintType)
enum class EPlayerStanceState : uint8
{
	Standing,
	Crouching
};

USTRUCT(BlueprintType)
struct FPlayerMovementSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	float WalkSpeed;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	float SprintSpeed;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	UCurveVector* MovementCurve;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	UCurveFloat* RotationRateCurve;

	float GetSpeedForGait(const EPlayerGaitState Gait) const
	{
		switch (Gait)
		{
		case EPlayerGaitState::Sprinting:
			return SprintSpeed;
		case EPlayerGaitState::Walking:
			return WalkSpeed;
		default:
			return SprintSpeed;
		}
	}
};


USTRUCT(BlueprintType)
struct FPlayerMovementModel : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	FPlayerMovementSettings Standing;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Movement")
	FPlayerMovementSettings Crouching;
};


UCLASS()
class MONATY_API AMonatyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonatyCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	FORCEINLINE class UMonatyCharacterMovementComponent* GetMyMovementComponent() const
	{
		return MyCharacterMovementComponent;
	}

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void SetMovementModel();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetStance(EPlayerStanceState NewStance);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetGait(EPlayerGaitState NewGait);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMovementState(EPlayerMovementState NewMovement);

	UFUNCTION(BlueprintCallable, Category = "Movement|Rotation")
	void SetLocationAndTargetRotation(FVector NewLocation, FRotator NewRotator);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool HasMovementInput() const { return bHasMovementInput; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetHasMovementInput(bool bNewHasMovementInput);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	EPlayerGaitState GetAllowedGait() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	EPlayerGaitState GetActualGait() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool CanSprint() const;

	UFUNCTION(BlueprintCallable, Category = "Rotation")
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Rotation")
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Rotation")
	float CalculateGroundedRotationRate() const;

	UFUNCTION(BlueprintGetter, Category = "Essential")
	FVector GetAcceleration() const { return Acceleration; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetAcceleration(const FVector& NewAcceleration);

	UFUNCTION(BlueprintGetter, Category = "Essential")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetIsMoving(bool bNewIsMoving);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	FVector GetMovementInput() const;

	UFUNCTION(BlueprintGetter, Category = "Essential")
	float GetMovementInputAmount() const { return MovementInputAmount; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetMovementInputAmount(float NewMovementInputAmount);

	UFUNCTION(BlueprintGetter, Category = "Essential")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	FRotator GetAimingRotation() const { return AimingRotation; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	float GetAimYawRate() const { return AimYawRate; }

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetAimYawRate(float NewAimYawRate);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void GetControlForwardRightVector(FVector& Forward, FVector& Right) const;
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FPlayerMovementSettings GetCurrentMovementSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void UpdateCharacterMovement();

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void SetEssentialValues(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void UpdateGroundedRotation(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Essential")
	void UpdateInAirRotation(float DeltaTime);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Input mappings */
	void ForwardBackwardInput(float Value);
	void LeftRightInput(float Value);
	void LookUpDownInput(float Value);
	void LookLeftRightInput(float Value);
	void SprintPressedAction();
	void SprintReleasedAction();
	void JumpPressedAction();
	void JumpReleasedAction();
	void StancePressedAction();
	void StanceReleasedAction();

	void PlaceModeAction();

	/** State changes */
	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0);
	void OnMovementStateChanged(EPlayerMovementState PreviousState);
	void OnStanceChanged(EPlayerStanceState PreviousStance);
	void OnGaitChanged(EPlayerGaitState PreviousGait);
	void OnJumped_Implementation() override;
	void Landed(const FHitResult& Hit) override;

	// Timelines
	UPROPERTY()
	FTimeline StanceTimeline;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	
	/* Components */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Components")
	UMonatyCharacterMovementComponent* MyCharacterMovementComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Components")
	UPlaceablesComponent* PlaceablesComponent;

	/* Properties */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Parameters|Essential|Curves")
	UCurveFloat* StanceCurve;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|State")
	EPlayerMovementState CurrentMovementState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|State")
	EPlayerMovementState PreviousMovementState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|Gait")
	EPlayerGaitState CurrentGaitState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|Gait")
	EPlayerGaitState PreviousGaitState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|Stance")
	EPlayerStanceState CurrentStanceState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement|Stance")
	EPlayerStanceState PreviousStanceState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parameters|Input")
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parameters|Input")
	float LookLeftRightRate = 1.25f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Rotation")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Rotation")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Rotation")
	FRotator AimingRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Rotation")
	float PreviousAimYaw = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Movement")
	FVector PreviousVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	FRotator LastMovementInputRotation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	float AimYawRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	float EasedMaxAcceleration = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	FVector CurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Parameters|Essential")
	FRotator ControlRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Parameters|Movement")
	FDataTableRowHandle MovementModel;

	UPROPERTY(BlueprintReadOnly, Category = "Parameters|Movement")
	FPlayerMovementModel PlayerMovementModel;
};
