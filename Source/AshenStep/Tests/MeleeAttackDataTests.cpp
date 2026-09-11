#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimMontage.h"
#include "Combat/MeleeAttackData.h"

namespace AshenStep::MeleeAttackDataTests
{
constexpr EAutomationTestFlags TestFlags =
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackSafeDefaultsTest,
	"AshenStep.MeleeAttack.Configuration.SafeDefaults",
	AshenStep::MeleeAttackDataTests::TestFlags)

bool FMeleeAttackSafeDefaultsTest::RunTest(const FString& Parameters)
{
	const FMeleeAttackData Attack;

	TestNull(TEXT("An unconfigured attack has no montage"), Attack.AttackMontage);
	TestEqual(TEXT("An unconfigured attack deals no damage"), Attack.DamageAmount, 0.0f);
	TestFalse(TEXT("An unconfigured attack is invalid"), Attack.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackInvalidConfigurationTest,
	"AshenStep.MeleeAttack.Configuration.InvalidValues",
	AshenStep::MeleeAttackDataTests::TestFlags)

bool FMeleeAttackInvalidConfigurationTest::RunTest(const FString& Parameters)
{
	FMeleeAttackData ValidBaseline;
	ValidBaseline.AttackMontage = NewObject<UAnimMontage>();
	ValidBaseline.DamageAmount = 25.0f;
	ValidBaseline.TraceRadius = 12.0f;
	ValidBaseline.TraceStartSocket = TEXT("weapon_base");
	ValidBaseline.TraceEndSocket = TEXT("weapon_tip");
	TestTrue(TEXT("A fully configured attack is valid"), ValidBaseline.IsValid());

	FMeleeAttackData Attack = ValidBaseline;
	Attack.AttackMontage = nullptr;
	TestFalse(TEXT("Missing montage rejects the attack"), Attack.IsValid());

	Attack = ValidBaseline;
	Attack.DamageAmount = 0.0f;
	TestFalse(TEXT("Nonpositive damage rejects the attack"), Attack.IsValid());

	Attack = ValidBaseline;
	Attack.TraceRadius = 0.0f;
	TestFalse(TEXT("Nonpositive trace radius rejects the attack"), Attack.IsValid());

	Attack = ValidBaseline;
	Attack.TraceStartSocket = NAME_None;
	TestFalse(TEXT("An empty start socket rejects the attack"), Attack.IsValid());

	Attack = ValidBaseline;
	Attack.TraceEndSocket = NAME_None;
	TestFalse(TEXT("An empty end socket rejects the attack"), Attack.IsValid());
	return true;
}

#endif
