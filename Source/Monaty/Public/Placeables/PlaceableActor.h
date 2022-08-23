// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlaceableActor.generated.h"

UCLASS()
class MONATY_API APlaceableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaceableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeCollisionResponses() const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* Components */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Components")
	USceneComponent* PlaceableRootComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Components")
	UStaticMeshComponent* PlaceableMeshComponent;
};
