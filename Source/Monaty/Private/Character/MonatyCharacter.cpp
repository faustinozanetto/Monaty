// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MonatyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/MonatyCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMonatyCharacter::AMonatyCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UMonatyCharacterMovementComponent>(
		CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = 0;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, TEXT("SpringEndPoint"));

	PlaceablesComponent = CreateDefaultSubobject<UPlaceablesComponent>(TEXT("Placeables"));
	AddOwnedComponent(PlaceablesComponent);
}

// Called when the game starts or when spawned
void AMonatyCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Make sure the mesh and animbp update after the CharacterBP to ensure it gets the most recent values.
	GetMesh()->AddTickPrerequisiteActor(this);
	// Set the Movement Model
	SetMovementModel();
	// Setup timelines
	if (StanceCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName(TEXT("StanceTimelineProgress")));

		StanceTimeline.AddInterpFloat(StanceCurve, TimelineProgress);
	}
}

// Called every frame
void AMonatyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set required values
	SetEssentialValues(DeltaTime);

	if (CurrentMovementState == EPlayerMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
	}
	else if (CurrentMovementState == EPlayerMovementState::InAir)
	{
		UpdateInAirRotation(DeltaTime);
	}

	// Update timelines.
	StanceTimeline.TickTimeline(DeltaTime);

	// Cache values
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

// Called to bind functionality to input
void AMonatyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Movement axis.
	PlayerInputComponent->BindAxis("MoveForward/Backwards", this, &AMonatyCharacter::ForwardBackwardInput);
	PlayerInputComponent->BindAxis("MoveRight/Left", this, &AMonatyCharacter::LeftRightInput);
	PlayerInputComponent->BindAxis("LookUp/Down", this, &AMonatyCharacter::LookUpDownInput);
	PlayerInputComponent->BindAxis("LookLeft/Right", this, &AMonatyCharacter::LookLeftRightInput);

	// Sprinting
	PlayerInputComponent->BindAction("SprintAction", IE_Pressed, this, &AMonatyCharacter::SprintPressedAction);
	PlayerInputComponent->BindAction("SprintAction", IE_Released, this, &AMonatyCharacter::SprintReleasedAction);

	// Jumping
	PlayerInputComponent->BindAction("JumpAction", IE_Pressed, this, &AMonatyCharacter::JumpPressedAction);
	PlayerInputComponent->BindAction("JumpAction", IE_Released, this, &AMonatyCharacter::JumpReleasedAction);

	// Stance AKA crouching
	PlayerInputComponent->BindAction("StanceAction", IE_Pressed, this, &AMonatyCharacter::StancePressedAction);
	PlayerInputComponent->BindAction("StanceAction", IE_Released, this, &AMonatyCharacter::StanceReleasedAction);

	// Placeables
	PlayerInputComponent->BindAction("PlaceableAction", IE_Pressed, this, &AMonatyCharacter::PlaceModeAction);
}

void AMonatyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UMonatyCharacterMovementComponent>(Super::GetMovementComponent());
}

void AMonatyCharacter::SetMovementModel()
{
	const FString ContextString = GetFullName();
	// The struct is not null, so we set it to the current player movement model.
	if (const FPlayerMovementModel* OutRow =
		MovementModel.DataTable->FindRow<FPlayerMovementModel>(MovementModel.RowName, ContextString))
	{
		PlayerMovementModel = *OutRow;
	}
}

void AMonatyCharacter::SetStance(EPlayerStanceState NewStance)
{
	if (CurrentStanceState != NewStance)
	{
		PreviousStanceState = CurrentStanceState;
		CurrentStanceState = NewStance;
		OnStanceChanged(PreviousStanceState);
	}
}

void AMonatyCharacter::SetGait(EPlayerGaitState NewGait)
{
	if (CurrentGaitState != NewGait)
	{
		PreviousGaitState = CurrentGaitState;
		CurrentGaitState = NewGait;
		OnGaitChanged(PreviousGaitState);
	}
}

void AMonatyCharacter::SetMovementState(EPlayerMovementState NewMovement)
{
	if (CurrentMovementState != NewMovement)
	{
		PreviousMovementState = CurrentMovementState;
		CurrentMovementState = NewMovement;
		OnMovementStateChanged(PreviousMovementState);
	}
}

void AMonatyCharacter::SetLocationAndTargetRotation(FVector NewLocation, FRotator NewRotator)
{
	SetActorLocationAndRotation(NewLocation, NewRotator);
	TargetRotation = NewRotator;
}

void AMonatyCharacter::SetHasMovementInput(bool bNewHasMovementInput)
{
	bHasMovementInput = bNewHasMovementInput;
}

EPlayerGaitState AMonatyCharacter::GetAllowedGait() const
{
	// Calculate the Allowed Gait. This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc. For example,
	// if you wanted to force the character into a walking state while indoors, this could be done here.
	if (CurrentStanceState == EPlayerStanceState::Standing)
	{
		if (CurrentGaitState != EPlayerGaitState::Walking)
		{
			return CanSprint() ? EPlayerGaitState::Sprinting : EPlayerGaitState::None;
		}
	}
	return CurrentGaitState;
}

EPlayerGaitState AMonatyCharacter::GetActualGait() const
{
	// Get the Actual Gait. This is calculated by the actual movement of the character,  and so it can be different
	// from the desired gait or allowed gait. For instance, if the Allowed Gait becomes walking,
	// the Actual gait will still be running untill the character decelerates to the walking speed.
	//const float LocWalkSpeed = MyCharacterMovementComponent->CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = MyCharacterMovementComponent->CurrentMovementSettings.SprintSpeed;
	if (Speed > LocRunSpeed + 10.0f)
	{
		return EPlayerGaitState::Sprinting;
	}
	return EPlayerGaitState::Walking;
}

bool AMonatyCharacter::CanSprint() const
{
	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration
	// (input) rotation. If the character is in the Looking Rotation mode, only allow sprinting if there is full
	// movement input and it is faced forward relative to the camera + or - 50 degrees.
	if (!bHasMovementInput)
	{
		return false;
	}
	const bool bValidInputAmount = MovementInputAmount > 0.9f;
	const FRotator AccRot = Acceleration.ToOrientationRotator();
	FRotator Delta = AccRot - AimingRotation;
	Delta.Normalize();
	return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
}

void AMonatyCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	// Prevent the character from rotating past a certain angle.
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void AMonatyCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed,
                                               float DeltaTime)
{
	// Interpolate the Target Rotation for extra smooth rotation behavior
	TargetRotation =
		FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

float AMonatyCharacter::CalculateGroundedRotationRate() const
{
	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation
	// rates for each speed. Increase the speed if the camera is rotating quickly for more responsive rotation.
	const float MappedSpeedVal = GetMyMovementComponent()->GetMappedSpeed();
	const float CurveVal =
		MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped(FVector2f{0.0f, 300.0f}, FVector2f{1.0f, 3.0f},
	                                                                  AimYawRate);
	return CurveVal * ClampedAimYawRate;
}

void AMonatyCharacter::SetAcceleration(const FVector& NewAcceleration)
{
	Acceleration = (NewAcceleration != FVector::ZeroVector || IsLocallyControlled())
		               ? NewAcceleration
		               : Acceleration / 2;
}

void AMonatyCharacter::SetIsMoving(bool bNewIsMoving)
{
	bIsMoving = bNewIsMoving;
}

FVector AMonatyCharacter::GetMovementInput() const
{
	return Acceleration;
}

void AMonatyCharacter::SetMovementInputAmount(float NewMovementInputAmount)
{
	MovementInputAmount = NewMovementInputAmount;
}

void AMonatyCharacter::SetSpeed(float NewSpeed)
{
	Speed = NewSpeed;
}

void AMonatyCharacter::SetAimYawRate(float NewAimYawRate)
{
	AimYawRate = NewAimYawRate;
}

void AMonatyCharacter::GetControlForwardRightVector(FVector& Forward, FVector& Right) const
{
	const FRotator ControlRot(0.0f, AimingRotation.Yaw, 0.0f);
	Forward = GetInputAxisValue("MoveForward/Backwards") * UKismetMathLibrary::GetForwardVector(ControlRot);
	Right = GetInputAxisValue("MoveRight/Left") * UKismetMathLibrary::GetRightVector(ControlRot);
}

FPlayerMovementSettings AMonatyCharacter::GetCurrentMovementSettings() const
{
	if (CurrentStanceState == EPlayerStanceState::Standing)
	{
		return PlayerMovementModel.Standing;
	}
	if (CurrentStanceState == EPlayerStanceState::Crouching)
	{
		return PlayerMovementModel.Crouching;
	}

	return PlayerMovementModel.Standing;
}

void AMonatyCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	const EPlayerGaitState AllowedGait = GetAllowedGait();

	// Get the Current Movement Settings and pass it through to the movement component.
	MyCharacterMovementComponent->SetMovementSettings(GetCurrentMovementSettings());

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	const float NewMaxSpeed = MyCharacterMovementComponent->CurrentMovementSettings.GetSpeedForGait(AllowedGait);
	MyCharacterMovementComponent->SetMaxWalkingSpeed(NewMaxSpeed);
}

void AMonatyCharacter::SetEssentialValues(float DeltaTime)
{
	CurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
	ControlRotation = GetControlRotation();
	EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();

	// Interp AimingRotation to current control rotation for smooth character rotation movement. Decrease InterpSpeed
	// for slower but smoother movement.
	AimingRotation = FMath::RInterpTo(AimingRotation, ControlRotation, DeltaTime, 30);

	// These values represent how the capsule is moving as well as how it wants to move, and therefore are essential
	// for any data driven animation system. They are also used throughout the system for various functions,
	// so I found it is easiest to manage them all in one place.

	const FVector CurrentVel = GetVelocity();

	// Set the amount of Acceleration.
	SetAcceleration((CurrentVel - PreviousVelocity) / DeltaTime);

	// Determine if the character is moving by getting it's speed. The Speed equals the length of the horizontal (x y)
	// velocity, so it does not take vertical movement into account. If the character is moving, update the last
	// velocity rotation. This value is saved because it might be useful to know the last orientation of movement
	// even after the character has stopped.
	SetSpeed(CurrentVel.Size2D());
	SetIsMoving(Speed > 1.0f);

	if (bIsMoving)
	{
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that
	// it has a range of 0-1, 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	SetMovementInputAmount(CurrentAcceleration.Size() / EasedMaxAcceleration);
	SetHasMovementInput(MovementInputAmount > 0.0f);
	if (bHasMovementInput)
	{
		LastMovementInputRotation = CurrentAcceleration.ToOrientationRotator();
	}
	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right.
	SetAimYawRate(FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime));
}

void AMonatyCharacter::UpdateGroundedRotation(float DeltaTime)
{
	const bool bCanUpdateMovingRot = (bIsMoving && bHasMovementInput || Speed > 150.0f);
	if (bCanUpdateMovingRot)
	{
		const float GroundedRotationRate = CalculateGroundedRotationRate();
		const float YawValue = AimingRotation.Yaw;
		SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
	}
	else
	{
		// Not moving.
		LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
	}
}

void AMonatyCharacter::UpdateInAirRotation(float DeltaTime)
{
	// Velocity / Looking Direction Rotation
	SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
}

void AMonatyCharacter::ForwardBackwardInput(float Value)
{
	if (CurrentMovementState == EPlayerMovementState::Grounded || CurrentMovementState ==
		EPlayerMovementState::InAir)
	{
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetForwardVector(DirRotator), Value);
	}
}

void AMonatyCharacter::LeftRightInput(float Value)
{
	if (CurrentMovementState == EPlayerMovementState::Grounded || CurrentMovementState ==
		EPlayerMovementState::InAir)
	{
		// Default camera relative movement behavior
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetRightVector(DirRotator), Value);
	}
}

void AMonatyCharacter::LookUpDownInput(float Value)
{
	AddControllerPitchInput(LookUpDownRate * Value);
}

void AMonatyCharacter::LookLeftRightInput(float Value)
{
	AddControllerYawInput(LookLeftRightRate * Value);
}

void AMonatyCharacter::SprintPressedAction()
{
	SetGait(EPlayerGaitState::Sprinting);
}

void AMonatyCharacter::SprintReleasedAction()
{
	SetGait(EPlayerGaitState::Walking);
}

void AMonatyCharacter::JumpPressedAction()
{
	if (CurrentMovementState == EPlayerMovementState::Grounded)
	{
		if (CurrentStanceState == EPlayerStanceState::Standing)
		{
			Jump();
		}
		else if (CurrentStanceState == EPlayerStanceState::Crouching)
		{
			UnCrouch();
		}
	}
}

void AMonatyCharacter::JumpReleasedAction()
{
	StopJumping();
}

void AMonatyCharacter::StancePressedAction()
{
	if (GetCharacterMovement()->IsMovingOnGround() && !(CurrentGaitState == EPlayerGaitState::Sprinting))
	{
		SetStance(EPlayerStanceState::Crouching);
		StanceTimeline.Play();
	}
}

void AMonatyCharacter::StanceReleasedAction()
{
	if (GetCharacterMovement()->IsMovingOnGround() && !(CurrentGaitState == EPlayerGaitState::Sprinting))
	{
		SetStance(EPlayerStanceState::Standing);
		StanceTimeline.Reverse();
	}
}

void AMonatyCharacter::PlaceModeAction()
{
	if (PlaceablesComponent)
	{
		// We toggle the place mode.
		PlaceablesComponent->bIsPlacing = !PlaceablesComponent->bIsPlacing;
	}
}

void AMonatyCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	// Use the Character Movement Mode changes to set the Movement States to the right values. This allows you to have
	// a custom set of movement states but still use the functionality of the default character movement component.
	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EPlayerMovementState::Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EPlayerMovementState::InAir);
	}
}

void AMonatyCharacter::OnMovementStateChanged(EPlayerMovementState PreviousState)
{
	if (CurrentMovementState == EPlayerMovementState::InAir)
	{
		// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
		InAirRotation = GetActorRotation();
		if (CurrentStanceState == EPlayerStanceState::Crouching)
		{
			UnCrouch();
		}
	}
}

void AMonatyCharacter::OnStanceChanged(EPlayerStanceState PreviousStance)
{
	// This is the place to make the animbp state change call
}

void AMonatyCharacter::OnGaitChanged(EPlayerGaitState PreviousGait)
{
	// This is the place to make the animbp state change call
}

void AMonatyCharacter::OnJumped_Implementation()
{
	// Set the new In Air Rotation to the velocity rotation if speed is greater than 100.
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();
}

void AMonatyCharacter::Landed(const FHitResult& Hit)
{
	// Todo.
}
