// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageContext.generated.h"

UENUM (BlueprintType)
enum class EDamageType : uint8
{
	None UMETA(DisplayName = "None"),
	Fire UMETA(DisplayName = "Fire"),
	Blunt UMETA(DisplayName = "Blunt"),
	Slice UMETA(DisplayName = "Slice")
};

USTRUCT (BlueprintType)
struct ASHENSTEP_API FDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	float DamageAmount = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> DamageInstigator = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> DamageSource = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	FVector DamageHitLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage|Type")
	EDamageType DamageType = EDamageType::None;

	FDamageContext() = default;
};