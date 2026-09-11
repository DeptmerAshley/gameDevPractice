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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackBusyInputTest,
	"AshenStep.MeleeAttack.Model.BusyInputPreservesEveryPhase",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackBusyInputTest::RunTest(const FString& Parameters)
{
	FMeleeAttackModel Attack;
	TestTrue(TEXT("Initial press starts a swing"), Attack.TryStartAttack());
	const EMeleeState BusyPhases[] = { EMeleeState::WindUp, EMeleeState::Active, EMeleeState::Recovery };
	for (const EMeleeState Phase : BusyPhases)
	{
		if (Phase == EMeleeState::Active)
		{
			TestTrue(TEXT("Authored signal opens the window"), Attack.TryAttack());
		}
		else if (Phase == EMeleeState::Recovery)
		{
			TestTrue(TEXT("Authored signal closes the window"), Attack.TryEndAttack());
		}
		for (int32 Press = 0; Press < 5; ++Press)
		{
			TestFalse(TEXT("Busy phase reports that a new attack cannot start"), Attack.CanStartAttack());
			TestFalse(TEXT("Repeated input cannot restart a busy attack"), Attack.TryStartAttack());
			TestEqual(TEXT("Rejected input preserves the current phase"), Attack.GetState(), Phase);
			TestEqual(TEXT("Rejected input does not change hit permission"), Attack.CanRegisterHits(), Phase == EMeleeState::Active);
		}
	}
	TestTrue(TEXT("Recovery still completes after input spam"), Attack.TryEndRecovery());
	TestTrue(TEXT("A fresh press after completion starts another swing"), Attack.TryStartAttack());
	TestEqual(TEXT("The next swing starts in wind-up"), Attack.GetState(), EMeleeState::WindUp);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeAttackRestartAfterInterruptionTest,
	"AshenStep.MeleeAttack.Model.RestartAfterInterruptionInEveryPhase",
	AshenStep::MeleeAttackTests::TestFlags)

bool FMeleeAttackRestartAfterInterruptionTest::RunTest(const FString& Parameters)
{
	const EMeleeState BusyPhases[] = { EMeleeState::WindUp, EMeleeState::Active, EMeleeState::Recovery };
	for (const EMeleeState Phase : BusyPhases)
	{
		FMeleeAttackModel Attack;
		Attack.TryStartAttack();
		if (Phase != EMeleeState::WindUp) { Attack.TryAttack(); }
		if (Phase == EMeleeState::Recovery) { Attack.TryEndAttack(); }
		TestEqual(TEXT("Arrange the intended interruption phase"), Attack.GetState(), Phase);

		TestTrue(TEXT("Interrupt reports ending an attack"), Attack.Interrupt());
		TestEqual(TEXT("Interruption immediately returns to ready"), Attack.GetState(), EMeleeState::Ready);
		TestFalse(TEXT("Interruption immediately disables hits"), Attack.CanRegisterHits());
		TestTrue(TEXT("Interruption permits a fresh attack"), Attack.CanStartAttack());
		TestFalse(TEXT("Duplicate cleanup reports no remaining attack"), Attack.Interrupt());
		TestEqual(TEXT("Duplicate cleanup preserves ready"), Attack.GetState(), EMeleeState::Ready);
		TestFalse(TEXT("A late close signal while idle is harmless"), Attack.TryEndAttack());
		TestFalse(TEXT("A late completion signal while idle is harmless"), Attack.TryEndRecovery());

		TestTrue(TEXT("A new press is accepted after cleanup"), Attack.TryStartAttack());
		TestFalse(TEXT("The new wind-up does not inherit an active hit window"), Attack.CanRegisterHits());
		TestTrue(TEXT("The new swing can open its window"), Attack.TryAttack());
		TestTrue(TEXT("The new swing can close its window"), Attack.TryEndAttack());
		TestTrue(TEXT("The new swing can complete normally"), Attack.TryEndRecovery());
	}
	return true;
}

#endif
