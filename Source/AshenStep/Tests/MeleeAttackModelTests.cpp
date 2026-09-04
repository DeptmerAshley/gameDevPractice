#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/MeleeAttackModel.h"

namespace AshenStep::MeleeAttackTests
{
constexpr EAutomationTestFlags TestFlags =
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackInitialStateTest,
	"AshenStep.MeleeAttack.Model.InitialState",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackInitialStateTest::RunTest(const FString& Parameters)
{
	const FMeleeAttackModel Attack;

	TestEqual(TEXT("A new attack is ready"), Attack.GetState(), EMeleeState::Ready);
	TestTrue(TEXT("A new attack can begin"), Attack.CanStartAttack());
	TestFalse(TEXT("A new attack cannot register hits"), Attack.CanRegisterHits());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackCompleteLifecycleTest,
	"AshenStep.MeleeAttack.Model.CompleteLifecycle",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackCompleteLifecycleTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel Attack;

	TestTrue(TEXT("A ready attack request succeeds"), Attack.TryStartAttack());
	TestEqual(TEXT("Starting enters wind-up"), Attack.GetState(), EMeleeState::WindUp);
	TestFalse(TEXT("Wind-up cannot register hits"), Attack.CanRegisterHits());

	TestTrue(TEXT("The authored begin-window signal succeeds during wind-up"), Attack.TryAttack());
	TestEqual(TEXT("Opening the window enters active"), Attack.GetState(), EMeleeState::Active);
	TestTrue(TEXT("Active attacks can register hits"), Attack.CanRegisterHits());

	TestTrue(TEXT("The authored end-window signal succeeds while active"), Attack.TryEndAttack());
	TestEqual(TEXT("Closing the window enters recovery"), Attack.GetState(), EMeleeState::Recovery);
	TestFalse(TEXT("Recovery cannot register hits"), Attack.CanRegisterHits());

	TestTrue(TEXT("The authored completion signal succeeds during recovery"), Attack.TryEndRecovery());
	TestEqual(TEXT("Completion returns to ready"), Attack.GetState(), EMeleeState::Ready);
	TestTrue(TEXT("Another attack may begin after completion"), Attack.CanStartAttack());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackRepeatedRequestTest,
	"AshenStep.MeleeAttack.Model.RepeatedAttackRequest",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackRepeatedRequestTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel Attack;
	Attack.TryStartAttack();

	TestFalse(TEXT("A second request during wind-up is rejected"), Attack.TryStartAttack());
	TestEqual(TEXT("A rejected request preserves wind-up"), Attack.GetState(), EMeleeState::WindUp);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackDuplicateSignalsTest,
	"AshenStep.MeleeAttack.Model.DuplicateSignals",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackDuplicateSignalsTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel Attack;
	Attack.TryStartAttack();
	Attack.TryAttack();

	TestFalse(TEXT("A duplicate begin-window signal is rejected"), Attack.TryAttack());
	TestEqual(TEXT("A duplicate begin signal preserves active"), Attack.GetState(), EMeleeState::Active);

	Attack.TryEndAttack();
	TestFalse(TEXT("A duplicate end-window signal is rejected"), Attack.TryEndAttack());
	TestEqual(TEXT("A duplicate end signal preserves recovery"), Attack.GetState(), EMeleeState::Recovery);

	Attack.TryEndRecovery();
	TestFalse(TEXT("A duplicate completion signal is rejected"), Attack.TryEndRecovery());
	TestEqual(TEXT("A duplicate completion preserves ready"), Attack.GetState(), EMeleeState::Ready);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackOutOfOrderSignalsTest,
	"AshenStep.MeleeAttack.Model.OutOfOrderSignals",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackOutOfOrderSignalsTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel Attack;

	TestFalse(TEXT("Ready rejects a begin-window signal"), Attack.TryAttack());
	TestFalse(TEXT("Ready rejects an end-window signal"), Attack.TryEndAttack());
	TestFalse(TEXT("Ready rejects an early completion signal"), Attack.TryEndRecovery());
	TestEqual(TEXT("Invalid ready signals preserve ready"), Attack.GetState(), EMeleeState::Ready);

	Attack.TryStartAttack();
	TestFalse(TEXT("Wind-up rejects an end-window signal"), Attack.TryEndAttack());
	TestFalse(TEXT("Wind-up rejects an early completion signal"), Attack.TryEndRecovery());
	TestEqual(TEXT("Invalid wind-up signals preserve wind-up"), Attack.GetState(), EMeleeState::WindUp);

	Attack.TryAttack();
	TestFalse(TEXT("Active rejects an early completion signal"), Attack.TryEndRecovery());
	TestEqual(TEXT("An early completion preserves active"), Attack.GetState(), EMeleeState::Active);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackInterruptionTest,
	"AshenStep.MeleeAttack.Model.Interruption",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackInterruptionTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel WindUpAttack;
	WindUpAttack.TryStartAttack();
	WindUpAttack.Interrupt();
	TestEqual(TEXT("Interrupting wind-up returns to ready"), WindUpAttack.GetState(), EMeleeState::Ready);

	FMeleeAttackModel ActiveAttack;
	ActiveAttack.TryStartAttack();
	ActiveAttack.TryAttack();
	ActiveAttack.Interrupt();
	TestEqual(TEXT("Interrupting active returns to ready"), ActiveAttack.GetState(), EMeleeState::Ready);
	TestFalse(TEXT("An interrupted active attack cannot register hits"), ActiveAttack.CanRegisterHits());

	FMeleeAttackModel RecoveryAttack;
	RecoveryAttack.TryStartAttack();
	RecoveryAttack.TryAttack();
	RecoveryAttack.TryEndAttack();
	RecoveryAttack.Interrupt();
	TestEqual(TEXT("Interrupting recovery returns to ready"), RecoveryAttack.GetState(), EMeleeState::Ready);

	FMeleeAttackModel ReadyAttack;
	ReadyAttack.Interrupt();
	TestEqual(TEXT("Interrupting ready is harmless"), ReadyAttack.GetState(), EMeleeState::Ready);
	return true;
}

#endif
