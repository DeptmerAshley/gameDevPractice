// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DamageTestTarget.h"

// Sets default values
ADamageTestTarget::ADamageTestTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADamageTestTarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADamageTestTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

