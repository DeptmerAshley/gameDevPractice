// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/MeleeAttackModel.h"
#include "Combat/MeleeAttackData.h"
#include "MeleeAttackComponent.generated.h"

class UWorld;
struct FCollisionShape;
struct FCollisionQueryParams;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASHENSTEP_API UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMeleeAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (AllowPrivateAccess = "true"))
	FMeleeAttackData MeleeAttackData;

	FMeleeAttackModel MeleeAttackModel;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool RequestMelee();
	EMeleeState GetState() const;
	bool CanRegisterHits() const;

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable, Category = "Melee")
	bool BeginAttackWindow();

	UFUNCTION(BlueprintCallable, Category = "Melee")
	bool EndAttackWindow();

	UPROPERTY(EditAnywhere, Category = "Melee|Debug")
	bool bDrawMeleeDebug = false;

private:
	void DrawWeaponDebug(UWorld* World, const FVector& BasePosition, const FVector& TipPosition) const;
	void SweepWeaponSample(UWorld* World, const FVector& Start, const FVector& End,
		const FCollisionShape& Shape, const FCollisionQueryParams& QueryParams, const TCHAR* SampleName) const;

	FVector LastWeaponBaseLoc = FVector::ZeroVector;
	FVector LastWeaponTipLoc = FVector::ZeroVector;
	bool bInitPositions = false;
	bool bReportedPositionFailure = false;
};
