#pragma once

#include "CoreMinimal.h"
#include "DamageContext.generated.h"

enum class EDamageType : uint8
{
	Fire,
	Blundt,
	Slice
};

USTRUCT (BlueprintType)
struct FDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float DamageAmount = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	FActor DamageInstigator = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	FActor DamageSource = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	FVector DamageHitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Damage|Type", meta = (AllowPrivateAccess = "true"))
	EDamageType DamageType;

	FDamageContext();
};