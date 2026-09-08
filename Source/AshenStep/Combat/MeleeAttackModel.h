// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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

	bool CanStartAttack() const;
	bool CanRegisterHits() const;

	bool TryStartAttack();
	bool TryAttack();
	bool TryEndAttack();
	bool TryEndRecovery();
	bool Interrupt();

	FMeleeAttackModel();
	~FMeleeAttackModel();

private:
	EMeleeState State = EMeleeState::Ready;
};
