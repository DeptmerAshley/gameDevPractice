#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Dash/DashAbilityModel.h"

namespace AshenStep::DashTests
{
constexpr EAutomationTestFlags TestFlags =
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

bool VectorsAreNearlyEqual(const FVector& Actual, const FVector& Expected)
{
	return Actual.Equals(Expected, KINDA_SMALL_NUMBER);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashInitialStateTest,
	"AshenStep.Dash.InitialState",
	AshenStep::DashTests::TestFlags)

bool FDashInitialStateTest::RunTest(const FString& Parameters)
{
	const FDashAbilityModel Dash;
	TestEqual(TEXT("A new dash starts ready"), Dash.GetState(), EDashState::Ready);
	TestTrue(TEXT("A grounded character can dash while ready"), Dash.CanDash(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashDirectionTest,
	"AshenStep.Dash.Direction",
	AshenStep::DashTests::TestFlags)

bool FDashDirectionTest::RunTest(const FString& Parameters)
{
	const FVector Forward = FVector::ForwardVector;
	const FVector Right = FVector::RightVector;

	const FVector ForwardResult = FDashAbilityModel::ResolveDashDirection(
		FVector2D(0.0f, 1.0f), Forward, Right);
	TestTrue(
		TEXT("Forward input resolves to camera forward"),
		AshenStep::DashTests::VectorsAreNearlyEqual(ForwardResult, Forward));

	const FVector RightResult = FDashAbilityModel::ResolveDashDirection(
		FVector2D(1.0f, 0.0f), Forward, Right);
	TestTrue(
		TEXT("Right input resolves to camera right"),
		AshenStep::DashTests::VectorsAreNearlyEqual(RightResult, Right));

	const FVector DiagonalResult = FDashAbilityModel::ResolveDashDirection(
		FVector2D(1.0f, 1.0f), Forward, Right);
	TestTrue(TEXT("Diagonal direction is normalized"), DiagonalResult.IsNormalized());
	TestTrue(
		TEXT("Diagonal direction combines forward and right"),
		AshenStep::DashTests::VectorsAreNearlyEqual(
			DiagonalResult,
			(Forward + Right).GetSafeNormal()));

	const FVector FallbackResult = FDashAbilityModel::ResolveDashDirection(
		FVector2D::ZeroVector, Forward, Right);
	TestTrue(
		TEXT("No input falls back to forward"),
		AshenStep::DashTests::VectorsAreNearlyEqual(FallbackResult, Forward));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashStartRulesTest,
	"AshenStep.Dash.StartRules",
	AshenStep::DashTests::TestFlags)

bool FDashStartRulesTest::RunTest(const FString& Parameters)
{
	FDashAbilityModel Dash;

	TestTrue(
		TEXT("A ready grounded dash request succeeds"),
		Dash.TryStartDash(
			FVector2D(0.0f, 1.0f),
			FVector::ForwardVector,
			FVector::RightVector,
			false));
	TestEqual(TEXT("A successful request enters Dashing"), Dash.GetState(), EDashState::Dashing);
	TestTrue(
		TEXT("The chosen direction is captured when the dash starts"),
		AshenStep::DashTests::VectorsAreNearlyEqual(
			Dash.GetDashDirection(),
			FVector::ForwardVector));
	TestFalse(
		TEXT("A repeated request while Dashing is rejected"),
		Dash.TryStartDash(
			FVector2D(1.0f, 0.0f),
			FVector::ForwardVector,
			FVector::RightVector,
			false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashTimingTest,
	"AshenStep.Dash.Timing",
	AshenStep::DashTests::TestFlags)

bool FDashTimingTest::RunTest(const FString& Parameters)
{
	FDashConfig Config;
	Config.Duration = 0.2f;
	Config.Cooldown = 1.0f;
	FDashAbilityModel Dash(Config);
	Dash.TryStartDash(
		FVector2D(0.0f, 1.0f),
		FVector::ForwardVector,
		FVector::RightVector,
		false);

	Dash.AdvanceTime(0.19f);
	TestEqual(TEXT("Dash remains active before duration expires"), Dash.GetState(), EDashState::Dashing);
	Dash.AdvanceTime(0.01f);
	TestEqual(TEXT("Dash enters cooldown at its duration"), Dash.GetState(), EDashState::Cooldown);
	TestFalse(TEXT("Dash cannot start during cooldown"), Dash.CanDash(false));

	Dash.AdvanceTime(0.99f);
	TestEqual(TEXT("Cooldown remains active before its boundary"), Dash.GetState(), EDashState::Cooldown);
	Dash.AdvanceTime(0.01f);
	TestEqual(TEXT("Cooldown returns to Ready at its boundary"), Dash.GetState(), EDashState::Ready);
	TestTrue(TEXT("Dash can start after cooldown"), Dash.CanDash(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashInterruptionAndAirRulesTest,
	"AshenStep.Dash.InterruptionAndAirRules",
	AshenStep::DashTests::TestFlags)

bool FDashInterruptionAndAirRulesTest::RunTest(const FString& Parameters)
{
	FDashAbilityModel GroundOnlyDash;
	TestFalse(TEXT("Ground-only dash rejects airborne use"), GroundOnlyDash.CanDash(true));

	FDashConfig AirDashConfig;
	AirDashConfig.bAllowAirDash = true;
	FDashAbilityModel AirDash(AirDashConfig);
	TestTrue(TEXT("Configured air dash allows airborne use"), AirDash.CanDash(true));

	FDashAbilityModel InterruptedDash;
	InterruptedDash.TryStartDash(
		FVector2D(0.0f, 1.0f),
		FVector::ForwardVector,
		FVector::RightVector,
		false);
	InterruptedDash.EndDashEarly();
	TestEqual(
		TEXT("Collision-style early termination enters cooldown"),
		InterruptedDash.GetState(),
		EDashState::Cooldown);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDashLargeFrameTest,
	"AshenStep.Dash.LargeFrame",
	AshenStep::DashTests::TestFlags)

	bool FDashLargeFrameTest::RunTest(const FString& Parameters)
{
	FDashConfig Config;
	Config.Duration = 0.2f;
	Config.Cooldown = 1.0f;

	FDashAbilityModel Dash(Config);
	Dash.TryStartDash(
		FVector2D(0.0f, 1.0f),
		FVector::ForwardVector,
		FVector::RightVector,
		false);

	Dash.AdvanceTime(1.2f);

	TestEqual(
		TEXT("A frame spanning dash and cooldown returns to Ready"),
		Dash.GetState(),
		EDashState::Ready);

	return true;
}

#endif
