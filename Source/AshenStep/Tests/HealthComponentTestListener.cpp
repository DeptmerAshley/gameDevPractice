#include "Tests/HealthComponentTestListener.h"

void UHealthComponentTestListener::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	++HealthChangedCount;
	LastCurrentHealth = CurrentHealth;
	LastMaxHealth = MaxHealth;
	EventOrder.Add(TEXT("HealthChanged"));
}

void UHealthComponentTestListener::HandleDamageReceived(const FDamageContext& DamageContext, float AppliedDamage)
{
	++DamageReceivedCount;
	LastDamageContext = DamageContext;
	LastAppliedDamage = AppliedDamage;
	EventOrder.Add(TEXT("DamageReceived"));
}

void UHealthComponentTestListener::HandleDeath(const FDamageContext& DamageContext)
{
	++DeathCount;
	LastDeathContext = DamageContext;
	EventOrder.Add(TEXT("Death"));
}
