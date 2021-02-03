// Fill out your copyright notice in the Description page of Project Settings.


#include "BalloonOawn.h"

// Sets default values
ABalloonOawn::ABalloonOawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABalloonOawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABalloonOawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABalloonOawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

