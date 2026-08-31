#include "Dash/DashAbilityModel.h"

FDashAbilityModel::FDashAbilityModel(const FDashConfig& InConfig)
	: Config(InConfig)
{
}

bool FDashAbilityModel::CanDash(bool bIsAirborne) const
{
	return false;
}

bool FDashAbilityModel::TryStartDash(
	const FVector2D& MovementInput,
	const FVector& ForwardDirection,
	const FVector& RightDirection,
	bool bIsAirborne)
{
	return false;
}

void FDashAbilityModel::AdvanceTime(float DeltaSeconds)
{
}

void FDashAbilityModel::EndDashEarly()
{
}

FVector FDashAbilityModel::ResolveDashDirection(
	const FVector2D& MovementInput,
	const FVector& ForwardDirection,
	const FVector& RightDirection)
{
	return FVector::ZeroVector;
}
