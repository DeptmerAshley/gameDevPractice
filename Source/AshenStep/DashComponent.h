// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dash/DashAbilityModel.h"
#include "DashComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHENSTEP_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDashComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Create room to cache a character with Transient UPROPERTIES
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

	FDashAbilityModel DashAbilityModel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float DashDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float DashDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true"))
	bool bAllowAirDash = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MomentumContribution = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxMomentumBonus = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tuning", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ExitSpeedRetention = 0.5f;

	float ActiveDashSpeed = 0.0f;
	bool bHadMovementInputAtDashStart = false;

	void HandleDashEnded(bool bWasBlocked);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool RequestDash(const FVector2D& MovementInput);
	bool IsDashing() const;

};
