#include "Tests/HealthComponentTestListener.h"

void UHealthComponentTestListener::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	++HealthChangedCount;
	LastCurrentHealth = CurrentHealth;
	LastMaxHealth = MaxHealth;
	EventOrder.Add(TEXT("HealthChanged"));
}

void UHealthComponentTestListener::HandleDeath()
{
	++DeathCount;
	EventOrder.Add(TEXT("Death"));
}
