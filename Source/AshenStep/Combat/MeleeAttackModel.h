// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAttackData.h"

enum class EMeleeState : uint8
{
	Ready,
	WindUp,
	Active,
	Recovery
};

class ASHENSTEP_API FMeleeAttackModel
{
public:
	EMeleeState GetState() const { return State; }
	// FMeleeAttackData GetAttackData() const { return MeleeAttackData; }

	bool TryStartAttack();
	bool TryAttack();
	bool TryEndAttack();

	FMeleeAttackModel();
	~FMeleeAttackModel();

private:
	EMeleeState State = EMeleeState::Ready;
	FMeleeAttackData MeleeAttackData;
};
