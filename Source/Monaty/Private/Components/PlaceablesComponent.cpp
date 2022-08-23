// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PlaceablesComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UPlaceablesComponent::UPlaceablesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UPlaceablesComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!InitPlaceablesComponents())
	{
		UE_LOG(LogTemp, Display,
		       TEXT("UPlaceablesComponent::InitPlaceablesComponents | Could not initialize component!"));
	}
}

// Called every frame
void UPlaceablesComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If we are in place mode, we update.
	if (bIsPlacing)
	{
		UpdatePlaceablePosition();
	}
}

bool UPlaceablesComponent::InitPlaceablesComponents()
{
	// Try and get the character owner.
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		PlayerCharacter = Character;
		// Try and get the character controller.
		if (APlayerController* Controller = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			PlayerController = Controller;
			// Enable tick once we setted up the references.
			SetComponentTickEnabled(true);
			return true;
		}
	}
	return false;
}

void UPlaceablesComponent::StartPlacingActors(FDataTableRowHandle PlaceableHandle)
{
	// Make sure we are not already in place mode.
	if (!bIsPlacing) return;

	// Validate placeable handle.
	if (!PlaceableHandle.IsNull())
	{
		// Find row.
		const FPlaceableData* PlaceableData = PlaceableHandle.DataTable->FindRow<FPlaceableData>(
			PlaceableHandle.RowName, "PLACEABLE");
		if (PlaceableData->StaticStruct())
		{
			CurrentPlaceableData = *PlaceableData;
			// Spawn new placeable.
			CreatePlaceableActor();
		}
	}
}

void UPlaceablesComponent::StopPlacingActors()
{
	// Make sure that we are in place mode.
	if (bIsPlacing)
	{
		bIsPlacing = false;
	}
}


void UPlaceablesComponent::CreatePlaceableActor()
{
	// Return if now valid class.
	if (!CurrentPlaceableData.PlaceableActorClass) return;
	// Destroy current placeable if for some reason the current one is valid.
	DestroyCurrentPlaceable();
	// Instantiate the placeable.
	const FTransform SpawnTransform = GetSpawnPlaceableTransform();
	const FActorSpawnParameters SpawnParameters = {
	};
	// Try and spawn placeable.
	if (APlaceableActor* PlaceableActor = GetWorld()->SpawnActor<APlaceableActor>(
		CurrentPlaceableData.PlaceableActorClass, SpawnTransform, SpawnParameters))
	{
		CurrentPlaceable = PlaceableActor;
		PlaceableTransform = FTransform::Identity;
	}
}

FHitResult UPlaceablesComponent::GetTraceHitResult() const
{
	const FVector StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector EndLocation = PlayerController->PlayerCameraManager->GetCameraRotation().Vector() * TraceDistance +
		StartLocation;
	const TArray<AActor*> IgnoreActors = {CurrentPlaceable, PlayerCharacter};
	FHitResult Result;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLocation, EndLocation,
	                                      UEngineTypes::ConvertToTraceType(ECC_Visibility),
	                                      false, IgnoreActors,
	                                      bDebugMode
		                                      ? EDrawDebugTrace::ForOneFrame
		                                      : EDrawDebugTrace::None, Result,
	                                      true);
	return Result;
}

void UPlaceablesComponent::UpdatePlaceablePosition()
{
	// If the current placeable is null, we return.
	if (!CurrentPlaceable) return;

	// Otherwise, calculate positions.
	FHitResult HitResult = GetTraceHitResult();
	FVector HitLocation = HitResult.TraceEnd;
	// Update placeable transform.
	FTransform NewPlaceableTransform = {GetPlaceableRotation(), GetFixedHitLocation(HitLocation), FVector::OneVector};
	UpdatePlaceableTransform(NewPlaceableTransform);
	// Update can place.
	bCanPlaceActor = HitResult.bBlockingHit;
	UpdatePlaceableMaterials(bCanPlaceActor);
}

void UPlaceablesComponent::DestroyCurrentPlaceable()
{
	// If the current placeable is valid, destroy it.
	if (CurrentPlaceable)
	{
		CurrentPlaceable->Destroy();
		CurrentPlaceable = nullptr;
	}
}

FVector UPlaceablesComponent::GetFixedHitLocation(FVector Location) const
{
	return {Location.X, Location.Y, Location.Z + PlaceableOffsetZ};
}

FRotator UPlaceablesComponent::GetPlaceableRotation() const
{
	// If the player controller is null return.
	if (!PlayerController) return {};
	return {0.0f, PlayerController->PlayerCameraManager->GetCameraRotation().Yaw + PlaceableRotationZ + 90.0f, 0.0f};
}

void UPlaceablesComponent::UpdatePlaceableTransform(const FTransform& Transform)
{
	// If they are almost not equal.
	if (!(FTransform::AreRotationsEqual(Transform, PlaceableTransform, 0.01f) &&
		FTransform::AreTranslationsEqual(Transform, PlaceableTransform, 0.01f) &&
		FTransform::AreScale3DsEqual(Transform, PlaceableTransform, 0.01f)))
	{
		PlaceableTransform = Transform;
		CurrentPlaceable->SetActorTransform(PlaceableTransform);
	}
}

void UPlaceablesComponent::UpdatePlaceableMaterials(bool bCanPlace) const
{
	// Make sure that the current placeable is valid.
	if (!CurrentPlaceable) return;
	// Loop through all its meshes.
	TArray<UStaticMeshComponent*> MeshComponents;
	CurrentPlaceable->GetComponents<UStaticMeshComponent>(MeshComponents);
	for (const auto MeshComponent : MeshComponents)
	{
		// Loop through all its materials.
		for (int MaterialIndex = 0; MaterialIndex < MeshComponent->GetMaterials().Num(); MaterialIndex++)
		{
			// Set material
			MeshComponent->SetMaterial(MaterialIndex, bCanPlace ? AllowPlaceMaterial : DenyPlaceMaterial);
		}
	}
}

void UPlaceablesComponent::ConstructPlaceableActor()
{
	// If there is no placeable, return.
	if (!CurrentPlaceable) return;
	// If we cannot place, return.
	if (!bCanPlaceActor) return;

	// Destroy placeable actor.
	DestroyCurrentPlaceable();
	// Create the placed actor.
	const FTransform SpawnTransform = PlaceableTransform;
	const FActorSpawnParameters SpawnParameters = {
	};
	// Try and spawn placeable.
	if (AActor* PlacedActor = GetWorld()->SpawnActor<AActor>(
		CurrentPlaceableData.PlacedActorClass, SpawnTransform, SpawnParameters))
	{
		UE_LOG(LogTemp, Display, TEXT("UPlaceablesComponent::ConstructPlaceableActor Successfully created Placed Actor"));
	}
}

FTransform UPlaceablesComponent::GetSpawnPlaceableTransform()
{
	return PlayerCharacter->GetActorTransform();
}
