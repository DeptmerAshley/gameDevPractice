// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Damage/DamageContext.h"
#include "MeleeAttackData.generated.h"

class UAnimMontage;
//class USoundBase;
//class UCameraShakeBase;

USTRUCT (BlueprintType)
struct ASHENSTEP_API FMeleeAttackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float DamageAmount = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	EDamageType DamageType = EDamageType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace", meta = (ClampMin = "0.0", Units = "cm"))
	float TraceRadius = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	FName TraceStartSocket = TEXT("weapon_base");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace")
	FName TraceEndSocket = TEXT("weapon_tip");
};
