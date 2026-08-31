#pragma once

#include "CoreMinimal.h"

enum class EDashState : uint8
{
	Ready,
	Dashing,
	Cooldown
};

struct FDashConfig
{
	float Duration = 0.2f;
	float Cooldown = 1.0f;
	float Distance;
	bool bAllowAirDash = false;
};

/**
 * Engine-independent dash rules.
 *
 * This model owns direction selection and state transitions only. A future
 * Actor Component will use it while Unreal's movement system owns collision
 * and physical movement.
 */
class ASHENSTEP_API FDashAbilityModel
{
public:
	explicit FDashAbilityModel(const FDashConfig& InConfig = FDashConfig());

	EDashState GetState() const { return State; }
	const FVector& GetDashDirection() const { return DashDirection; }
	bool CanDash(bool bIsAirborne) const;

	bool TryStartDash(
		const FVector2D& MovementInput,
		const FVector& ForwardDirection,
		const FVector& RightDirection,
		bool bIsAirborne);

	void AdvanceTime(float DeltaSeconds);
	void EndDashEarly();

	static FVector ResolveDashDirection(
		const FVector2D& MovementInput,
		const FVector& ForwardDirection,
		const FVector& RightDirection);

private:
	FDashConfig Config;
	EDashState State = EDashState::Ready;
	FVector DashDirection = FVector::ZeroVector;
	float StateElapsedTime = 0.0f;
};
