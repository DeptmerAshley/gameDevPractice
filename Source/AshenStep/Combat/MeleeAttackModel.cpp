// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MeleeAttackModel.h"

FMeleeAttackModel::FMeleeAttackModel()
{
}

FMeleeAttackModel::~FMeleeAttackModel()
{
}

bool FMeleeAttackModel::TryStartAttack()
{
	if (GetState() == EMeleeState::Ready)
	{
		State = EMeleeState::WindUp;
		return true;
	}
	return false;
}

bool FMeleeAttackModel::TryAttack()
{
	if (GetState() == EMeleeState::WindUp)
	{
		State = EMeleeState::Active;
		return true;
	}
	return false;
}

bool FMeleeAttackModel::TryEndAttack()
{
	if (GetState() == EMeleeState::Active)
	{
		State = EMeleeState::Recovery;
		return true;
	}
	return false;
}
