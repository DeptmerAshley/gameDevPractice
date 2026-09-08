// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MeleeAttackModel.h"

FMeleeAttackModel::FMeleeAttackModel()
{
}

FMeleeAttackModel::~FMeleeAttackModel()
{
}

bool FMeleeAttackModel::CanStartAttack() const
{
	if (GetState() == EMeleeState::Ready)
	{
		return true;
	}
	return false;
}

bool FMeleeAttackModel::CanRegisterHits() const
{
	if (GetState() == EMeleeState::Active)
	{
		return true;
	}
	return false;
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

bool FMeleeAttackModel::TryEndRecovery()
{
	if (GetState() == EMeleeState::Recovery)
	{
		State = EMeleeState::Ready;
		return true;
	}
	return false;
}

bool FMeleeAttackModel::EndAttackEarly()
{
	State = EMeleeState::Ready;
	return true;
}
 
