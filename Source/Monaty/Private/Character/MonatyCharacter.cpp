// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MonatyCharacter.h"

// Sets default values
AMonatyCharacter::AMonatyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMonatyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMonatyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMonatyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

