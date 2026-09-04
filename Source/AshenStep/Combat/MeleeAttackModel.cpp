// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MeleeAttackModel.h"

FMeleeAttackModel::FMeleeAttackModel()
{
}

FMeleeAttackModel::~FMeleeAttackModel()
{
}

bool FMeleeAttackModel::TryStartAttack(EMeleeState& State)
{
	if (State == EMeleeState::Ready)
	{
		return true;
	}
	return false;
}

bool FMeleeAttackModel::TryAttack(EMeleeState& State)
{
	if (TryStartAttack(State))
	{
		State = EMeleeState::WindUp;
		return true;
	}
	return false;
}
