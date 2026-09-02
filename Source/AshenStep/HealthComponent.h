// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Damage/DamageContext.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,
	float, CurrentHealth,
	float, MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDamageReceived,
	const FDamageContext&, DamageContext,
	float, AppliedDamage
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDeath,
	const FDamageContext&, DamageContext
);

UCLASS( ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent) )
class ASHENSTEP_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	float ApplyDamage(const FDamageContext& DamageContext);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	float ApplyDamageAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	// Properties for applying damage, death, and changes to health
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeath OnDeath;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;
};
