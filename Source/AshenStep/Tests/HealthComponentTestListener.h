#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "HealthComponentTestListener.generated.h"

UCLASS()
class UHealthComponentTestProxy : public UHealthComponent
{
	GENERATED_BODY()

public:
	void InitializeForTest() { BeginPlay(); }
};

UCLASS()
class UHealthComponentTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleDeath();

	int32 HealthChangedCount = 0;
	int32 DeathCount = 0;
	float LastCurrentHealth = -1.0f;
	float LastMaxHealth = -1.0f;
	TArray<FName> EventOrder;
};
