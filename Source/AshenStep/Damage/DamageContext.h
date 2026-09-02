// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageContext.generated.h"

UENUM (BlueprintType)
enum class EDamageType : uint8
{
	Fire UMETA(DisplayName = "Fire"),
	Blunt UMETA(DisplayName = "Blunt"),
	Slice UMETA(DisplayName = "Slice")
};

USTRUCT (BlueprintType)
struct ASHENSTEP_API FDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DamageAmount = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TObjectPtr<AActor> DamageInstigator = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TObjectPtr<AActor> DamageSource = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FVector DamageHitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Damage|Type")
	EDamageType DamageType = EDamageType::Blunt;

	FDamageContext() = default;
};