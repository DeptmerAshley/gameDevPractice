// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DamageTestTarget.h"
#include "HealthComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADamageTestTarget::ADamageTestTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	SetRootComponent(TargetMesh);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ADamageTestTarget::BeginPlay()
{
	Super::BeginPlay();
	
}

UHealthComponent* ADamageTestTarget::GetHealthComponent() const
{
	return HealthComponent;
}
