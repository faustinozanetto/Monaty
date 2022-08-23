// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/PlaceableActor.h"

// Sets default values
APlaceableActor::APlaceableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlaceableRootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetRootComponent(PlaceableRootComponent);

	PlaceableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("PlaceableMeshComponent");
	PlaceableMeshComponent->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void APlaceableActor::BeginPlay()
{
	Super::BeginPlay();
	// Set initial collision response.
	InitializeCollisionResponses();
	
}

// Called every frame
void APlaceableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlaceableActor::InitializeCollisionResponses() const
{
	PlaceableMeshComponent->SetCollisionProfileName("OverlapAll");
}
