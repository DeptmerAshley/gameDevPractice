// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set current health to max at start-up
	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	CurrentHealth = MaxHealth;
	
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

bool UHealthComponent::IsAlive() const
{
	return CurrentHealth > 0.0f;
}

float UHealthComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.0f || !IsAlive())
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth - Amount,
		0.0f,
		MaxHealth
	);

	const float AppliedDamage = PreviousHealth - CurrentHealth;

	const bool bJustDied =
		PreviousHealth > 0.0f &&
		CurrentHealth <= 0.0f;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (bJustDied == true)
	{
		OnDeath.Broadcast();
	}

	return AppliedDamage;
}

float UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.0f || !IsAlive())
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth + Amount,
		0.0f,
		MaxHealth
	);

	const float AppliedHealing = CurrentHealth - PreviousHealth;

	if (AppliedHealing > 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}

	return AppliedHealing;
}
