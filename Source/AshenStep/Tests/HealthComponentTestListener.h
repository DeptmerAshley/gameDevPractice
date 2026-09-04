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
	void HandleDamageReceived(const FDamageContext& DamageContext, float AppliedDamage);

	UFUNCTION()
	void HandleDeath(const FDamageContext& DamageContext);

	int32 HealthChangedCount = 0;
	int32 DamageReceivedCount = 0;
	int32 DeathCount = 0;
	float LastCurrentHealth = -1.0f;
	float LastMaxHealth = -1.0f;
	float LastAppliedDamage = -1.0f;
	FDamageContext LastDamageContext;
	FDamageContext LastDeathContext;
	TArray<FName> EventOrder;
};
