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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Placeable")
	TSubclassOf<AActor> PlacedActorClass;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MONATY_API UPlaceablesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlaceablesComponent();

	UFUNCTION(BlueprintCallable, Category="Placeables")
	void StartPlacingActors(FDataTableRowHandle PlaceableHandle);

	UFUNCTION(BlueprintCallable, Category="Placeables")
	void StopPlacingActors();

	UFUNCTION(BlueprintCallable,Category="Placeables")
	void ConstructPlaceableActor();

	UFUNCTION(BlueprintCallable,Category="Placeables")
	void RotatePlaceableLeft(float Value);

	UFUNCTION(BlueprintCallable,Category="Placeables")
	float RotatePlaceableRight(float Value);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool InitPlaceablesComponents();
	void CreatePlaceableActor();
	FHitResult GetTraceHitResult() const;
	void UpdatePlaceablePosition();
	void DestroyCurrentPlaceable();
	FVector GetFixedHitLocation(FVector Location) const;
	FRotator GetPlaceableRotation() const;
	void UpdatePlaceableTransform(const FTransform& Transform);
	void UpdatePlaceableMaterials(bool bCanPlace) const;

	FTransform GetSpawnPlaceableTransform();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/* Properties */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Character")
	ACharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Character")
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|States")
	bool bIsPlacing = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	bool bDebugMode = true;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	bool bCanPlaceActor = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	FPlaceableData CurrentPlaceableData = {};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	APlaceableActor* CurrentPlaceable = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Properties|Placeable")
	FTransform PlaceableTransform = {};

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Properties|Placeable")
	float TraceDistance = 5000.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Properties|Placeable")
	float PlaceableOffsetZ = -5.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Properties|Placeable")
	float PlaceableRotationZ = 0.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Properties|Placeable")
	UMaterialInterface* AllowPlaceMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Properties|Placeable")
	UMaterialInterface* DenyPlaceMaterial;
	
};
