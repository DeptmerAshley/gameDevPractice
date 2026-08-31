#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AshenStepCharacter.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HealthComponent.h"
#include "Tests/HealthComponentTestListener.h"

namespace AshenStep::Tests
{
class FHealthTestFixture
{
public:
	FHealthTestFixture()
	{
		Owner = GWorld->SpawnActor<AActor>();
		Health = NewObject<UHealthComponentTestProxy>(Owner);
		Owner->AddInstanceComponent(Health);
		Health->RegisterComponent();
		Health->AddToRoot();
		Health->InitializeForTest();
	}

	~FHealthTestFixture()
	{
		Health->RemoveFromRoot();
		GWorld->DestroyActor(Owner);
	}

	UHealthComponent* GetHealth() const { return Health; }

private:
	AActor* Owner = nullptr;
	UHealthComponentTestProxy* Health = nullptr;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHealthInitializationTest,
	"AshenStep.Health.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHealthInitializationTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	const UHealthComponent* Health = Fixture.GetHealth();

	TestNotNull(TEXT("Health component is created"), Health);
	TestEqual(TEXT("Current health starts at maximum"), Health->GetCurrentHealth(), Health->GetMaxHealth());
	TestEqual(TEXT("Default maximum health is 100"), Health->GetMaxHealth(), 100.0f);
	TestTrue(TEXT("Component starts alive"), Health->IsAlive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHealthDamageTest,
	"AshenStep.Health.Damage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHealthDamageTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();

	TestEqual(TEXT("Thirty damage is applied"), Health->ApplyDamage(30.0f), 30.0f);
	TestEqual(TEXT("Thirty damage leaves seventy health"), Health->GetCurrentHealth(), 70.0f);
	TestEqual(TEXT("Zero damage is ignored"), Health->ApplyDamage(0.0f), 0.0f);
	TestEqual(TEXT("Negative damage is ignored"), Health->ApplyDamage(-10.0f), 0.0f);
	TestEqual(TEXT("Invalid damage does not change health"), Health->GetCurrentHealth(), 70.0f);
	TestEqual(TEXT("Overkill returns only remaining health"), Health->ApplyDamage(1000.0f), 70.0f);
	TestEqual(TEXT("Overkill clamps health to zero"), Health->GetCurrentHealth(), 0.0f);
	TestFalse(TEXT("Zero health is dead"), Health->IsAlive());
	TestEqual(TEXT("Damage after death is ignored"), Health->ApplyDamage(10.0f), 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHealthHealingTest,
	"AshenStep.Health.Healing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHealthHealingTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();

	Health->ApplyDamage(30.0f);
	TestEqual(TEXT("Twenty healing is applied"), Health->Heal(20.0f), 20.0f);
	TestEqual(TEXT("Twenty healing leaves ninety health"), Health->GetCurrentHealth(), 90.0f);
	TestEqual(TEXT("Overhealing returns only missing health"), Health->Heal(50.0f), 10.0f);
	TestEqual(TEXT("Overhealing clamps at maximum"), Health->GetCurrentHealth(), 100.0f);
	TestEqual(TEXT("Healing at maximum is ignored"), Health->Heal(20.0f), 0.0f);
	TestEqual(TEXT("Zero healing is ignored"), Health->Heal(0.0f), 0.0f);
	TestEqual(TEXT("Negative healing is ignored"), Health->Heal(-10.0f), 0.0f);

	Health->ApplyDamage(100.0f);
	TestEqual(TEXT("Healing after death is ignored"), Health->Heal(20.0f), 0.0f);
	TestEqual(TEXT("Dead health remains zero"), Health->GetCurrentHealth(), 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHealthEventsTest,
	"AshenStep.Health.Events",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHealthEventsTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnHealthChanged.AddDynamic(Listener, &UHealthComponentTestListener::HandleHealthChanged);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);

	Health->ApplyDamage(30.0f);
	TestEqual(TEXT("Valid damage broadcasts one health change"), Listener->HealthChangedCount, 1);
	TestEqual(TEXT("Damage broadcasts current health"), Listener->LastCurrentHealth, 70.0f);
	TestEqual(TEXT("Damage broadcasts maximum health"), Listener->LastMaxHealth, 100.0f);

	Health->Heal(20.0f);
	TestEqual(TEXT("Valid healing broadcasts one health change"), Listener->HealthChangedCount, 2);
	Health->Heal(1000.0f);
	TestEqual(TEXT("Overhealing that changes health broadcasts"), Listener->HealthChangedCount, 3);
	Health->Heal(20.0f);
	TestEqual(TEXT("Healing at maximum does not broadcast"), Listener->HealthChangedCount, 3);

	Health->ApplyDamage(1000.0f);
	TestEqual(TEXT("Lethal damage broadcasts health change"), Listener->HealthChangedCount, 4);
	TestEqual(TEXT("Lethal damage broadcasts death once"), Listener->DeathCount, 1);
	TestEqual(TEXT("Health reaches zero before death event"), Listener->EventOrder[Listener->EventOrder.Num() - 2], FName(TEXT("HealthChanged")));
	TestEqual(TEXT("Death is the final lethal event"), Listener->EventOrder.Last(), FName(TEXT("Death")));

	Health->ApplyDamage(10.0f);
	Health->Heal(10.0f);
	TestEqual(TEXT("Post-death operations do not broadcast health changes"), Listener->HealthChangedCount, 4);
	TestEqual(TEXT("Post-death operations do not repeat death"), Listener->DeathCount, 1);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHealthCharacterAccessTest,
	"AshenStep.Health.CharacterAccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHealthCharacterAccessTest::RunTest(const FString& Parameters)
{
	const AAshenStepCharacter* CharacterCDO = GetDefault<AAshenStepCharacter>();
	TestNotNull(TEXT("Character default object exists"), CharacterCDO);
	TestNotNull(TEXT("Character getter returns its health component"), CharacterCDO->GetHealthComponent());
	TestEqual(
		TEXT("Repeated getter calls return the same component"),
		CharacterCDO->GetHealthComponent(),
		CharacterCDO->GetHealthComponent());
	return true;
}

#endif
