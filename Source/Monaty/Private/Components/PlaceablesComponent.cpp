// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PlaceablesComponent.h"

#include "GameFramework/Character.h"

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
		UE_LOG(LogTemp, Display, TEXT("UPlaceablesComponent::InitPlaceablesComponents | Could not initialize component!"));
	}
}

// Called every frame
void UPlaceablesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UPlaceablesComponent::InitPlaceablesComponents()
{
	// Try and get the character owner.
	if	(ACharacter* Character = Cast<ACharacter>(GetOwner()))
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

void UPlaceablesComponent::StartPlacingActors()
{
	// Make sure we are not already in place mode.
	if (!bIsPlacing)
	{
		bIsPlacing = true;
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
	// Make sure to delete the current placeable if it wasn't null.
	if (CurrentPlaceable)
	{
		CurrentPlaceable->Destroy();
	}
	// Instantiate the placeable.
	const FTransform SpawnTransform = GetSpawnPlaceableTransform();
	const FActorSpawnParameters SpawnParameters = {};
	// Try and spawn placeable.
	if (APlaceableActor* PlaceableActor = GetWorld()->SpawnActor<APlaceableActor>(CurrentPlaceableData.PlaceableActorClass, SpawnTransform, SpawnParameters))
	{
		CurrentPlaceable = PlaceableActor;
	}
}

FTransform UPlaceablesComponent::GetSpawnPlaceableTransform()
{
	
}

