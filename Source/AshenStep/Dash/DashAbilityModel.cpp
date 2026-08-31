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

	StateElapsedTime += DeltaSeconds;
	bool bProcessedTransition = true;

	while (bProcessedTransition)
	{
		bProcessedTransition = false;

		switch (State)
		{
		case EDashState::Ready:
			StateElapsedTime = 0.0f;
			break;

		case EDashState::Dashing:
			if (StateElapsedTime >= Config.Duration)
			{
				StateElapsedTime -= Config.Duration;
				State = EDashState::Cooldown;
				bProcessedTransition = true;
			}
			break;

		case EDashState::Cooldown:
			if (StateElapsedTime >= Config.Cooldown)
			{
				StateElapsedTime = 0.0f;
				State = EDashState::Ready;
				bProcessedTransition = true;
			}
			break;
		}
	}
}

void FDashAbilityModel::EndDashEarly()
{
	if (State != EDashState::Dashing)
	{
		return;
	}

	State = EDashState::Cooldown;
	StateElapsedTime = 0.0f;
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
