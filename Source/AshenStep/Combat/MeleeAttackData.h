// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Damage/DamageContext.h"
#include "MeleeAttackData.generated.h"

class UAnimMontage;
class USoundBase;
class UCameraShakeBase;

USTRUCT (BlueprintType)
struct ASHENSTEP_API FMeleeAttackData
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> Animation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float DamageAmount = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	EDamageType DamageType = EDamageType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float TraceRadius = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace", meta = (AllowPrivateAccess = "true"))
	FName TraceStartSocket = TEXT("weapon_base");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace", meta = (AllowPrivateAccess = "true"))
	FName TraceEndSocket = TEXT("weapon_base");
};
