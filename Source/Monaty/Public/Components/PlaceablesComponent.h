// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Placeables/PlaceableActor.h"
#include "Engine/DataTable.h"
#include "PlaceablesComponent.generated.h"

USTRUCT(BlueprintType)
struct FPlaceableData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Placeable")
	TSubclassOf<APlaceableActor> PlaceableActorClass;
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MONATY_API UPlaceablesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlaceablesComponent();

	UFUNCTION(BlueprintCallable,Category="Placeables")
	void StartPlacingActors();

	UFUNCTION(BlueprintCallable,Category="Placeables")
	void StopPlacingActors();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool InitPlaceablesComponents();
	void CreatePlaceableActor();

	FTransform GetSpawnPlaceableTransform();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Properties */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Character")
	ACharacter* PlayerCharacter;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Character")
	APlayerController* PlayerController;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|States")
	bool bIsPlacing = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	FPlaceableData CurrentPlaceableData = {};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	APlaceableActor* CurrentPlaceable = nullptr;
};
