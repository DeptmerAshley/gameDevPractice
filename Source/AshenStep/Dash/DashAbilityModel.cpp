#include "Dash/DashAbilityModel.h"

FDashAbilityModel::FDashAbilityModel(const FDashConfig& InConfig)
	: Config(InConfig)
{
}

bool FDashAbilityModel::CanDash(bool bIsAirborne) const
{
	if ((!bIsAirborne || Config.bAllowAirDash) && (State == EDashState::Ready))
	{
		return true;
	}
	return false;
}

bool FDashAbilityModel::TryStartDash(
	const FVector2D& MovementInput,
	const FVector& ForwardDirection,
	const FVector& RightDirection,
	bool bIsAirborne)
{
	if (!CanDash(bIsAirborne))
	{
		return false;
	}
	DashDirection = ResolveDashDirection(
		MovementInput,
		ForwardDirection,
		RightDirection);


	State = EDashState::Dashing;
	StateElapsedTime = 0.0f;
	return true;
}

void FDashAbilityModel::AdvanceTime(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f)
	{
		return;
	}

	switch (State)
	{
		case EDashState::Ready:
			break;

		case EDashState::Dashing:
			StateElapsedTime += DeltaSeconds;
			if (StateElapsedTime >= Config.Duration) 
			{
				State = EDashState::Cooldown;
				StateElapsedTime = 0.f;
			}
			break;

		case EDashState::Cooldown:
			StateElapsedTime += DeltaSeconds;
			if (StateElapsedTime >= Config.Cooldown)
			{
				State = EDashState::Ready;
				StateElapsedTime = 0.0f;
			}
			break;
	}
}

void FDashAbilityModel::EndDashEarly()
{
}

FVector FDashAbilityModel::ResolveDashDirection(
	const FVector2D& MovementInput,
	const FVector& ForwardDirection,
	const FVector& RightDirection)
{
	if (MovementInput.IsNearlyZero())
	{
		return ForwardDirection.GetSafeNormal();
	}

	const FVector DesiredDirection =
		(ForwardDirection * MovementInput.Y) +
		(RightDirection * MovementInput.X);

	return DesiredDirection.GetSafeNormal();
}
