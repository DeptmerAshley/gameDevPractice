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

	TestEqual(TEXT("Thirty damage is applied"), Health->ApplyDamageAmount(30.0f), 30.0f);
	TestEqual(TEXT("Thirty damage leaves seventy health"), Health->GetCurrentHealth(), 70.0f);
	TestEqual(TEXT("Zero damage is ignored"), Health->ApplyDamageAmount(0.0f), 0.0f);
	TestEqual(TEXT("Negative damage is ignored"), Health->ApplyDamageAmount(-10.0f), 0.0f);
	TestEqual(TEXT("Invalid damage does not change health"), Health->GetCurrentHealth(), 70.0f);
	TestEqual(TEXT("Overkill returns only remaining health"), Health->ApplyDamageAmount(1000.0f), 70.0f);
	TestEqual(TEXT("Overkill clamps health to zero"), Health->GetCurrentHealth(), 0.0f);
	TestFalse(TEXT("Zero health is dead"), Health->IsAlive());
	TestEqual(TEXT("Damage after death is ignored"), Health->ApplyDamageAmount(10.0f), 0.0f);
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

	Health->ApplyDamageAmount(30.0f);
	TestEqual(TEXT("Twenty healing is applied"), Health->Heal(20.0f), 20.0f);
	TestEqual(TEXT("Twenty healing leaves ninety health"), Health->GetCurrentHealth(), 90.0f);
	TestEqual(TEXT("Overhealing returns only missing health"), Health->Heal(50.0f), 10.0f);
	TestEqual(TEXT("Overhealing clamps at maximum"), Health->GetCurrentHealth(), 100.0f);
	TestEqual(TEXT("Healing at maximum is ignored"), Health->Heal(20.0f), 0.0f);
	TestEqual(TEXT("Zero healing is ignored"), Health->Heal(0.0f), 0.0f);
	TestEqual(TEXT("Negative healing is ignored"), Health->Heal(-10.0f), 0.0f);

	Health->ApplyDamageAmount(100.0f);
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
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);

	Health->ApplyDamageAmount(30.0f);
	TestEqual(TEXT("Valid damage broadcasts one health change"), Listener->HealthChangedCount, 1);
	TestEqual(TEXT("Damage broadcasts current health"), Listener->LastCurrentHealth, 70.0f);
	TestEqual(TEXT("Damage broadcasts maximum health"), Listener->LastMaxHealth, 100.0f);

	Health->Heal(20.0f);
	TestEqual(TEXT("Valid healing broadcasts one health change"), Listener->HealthChangedCount, 2);
	TestEqual(TEXT("Healing does not broadcast damage received"), Listener->DamageReceivedCount, 1);
	Health->Heal(1000.0f);
	TestEqual(TEXT("Overhealing that changes health broadcasts"), Listener->HealthChangedCount, 3);
	TestEqual(TEXT("Overhealing does not broadcast damage received"), Listener->DamageReceivedCount, 1);
	Health->Heal(20.0f);
	TestEqual(TEXT("Healing at maximum does not broadcast"), Listener->HealthChangedCount, 3);

	Health->ApplyDamageAmount(1000.0f);
	TestEqual(TEXT("Lethal damage broadcasts health change"), Listener->HealthChangedCount, 4);
	TestEqual(TEXT("Lethal damage broadcasts damage received"), Listener->DamageReceivedCount, 2);
	TestEqual(TEXT("Lethal damage broadcasts death once"), Listener->DeathCount, 1);
	TestEqual(TEXT("Health reaches zero before damage event"), Listener->EventOrder[Listener->EventOrder.Num() - 3], FName(TEXT("HealthChanged")));
	TestEqual(TEXT("Damage received precedes death"), Listener->EventOrder[Listener->EventOrder.Num() - 2], FName(TEXT("DamageReceived")));
	TestEqual(TEXT("Death is the final lethal event"), Listener->EventOrder.Last(), FName(TEXT("Death")));

	Health->ApplyDamageAmount(10.0f);
	Health->Heal(10.0f);
	TestEqual(TEXT("Post-death operations do not broadcast health changes"), Listener->HealthChangedCount, 4);
	TestEqual(TEXT("Post-death operations do not repeat death"), Listener->DeathCount, 1);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageContextDefaultTest,
	"AshenStep.Health.DamageContext.DefaultIsHarmless",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDamageContextDefaultTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnHealthChanged.AddDynamic(Listener, &UHealthComponentTestListener::HandleHealthChanged);
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);
	const FDamageContext DamageContext;

	TestEqual(TEXT("Default context applies no damage"), Health->ApplyDamage(DamageContext), 0.0f);
	TestEqual(TEXT("Default context leaves health unchanged"), Health->GetCurrentHealth(), 100.0f);
	TestEqual(TEXT("Default context does not broadcast health change"), Listener->HealthChangedCount, 0);
	TestEqual(TEXT("Default context does not broadcast damage received"), Listener->DamageReceivedCount, 0);
	TestEqual(TEXT("Default context does not broadcast death"), Listener->DeathCount, 0);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAmountDamageCompatibilityTest,
	"AshenStep.Health.DamageContext.AmountEntryBuildsDefaultContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAmountDamageCompatibilityTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);

	TestEqual(TEXT("Amount entry applies damage"), Health->ApplyDamageAmount(20.0f), 20.0f);
	TestEqual(TEXT("Amount entry broadcasts once"), Listener->DamageReceivedCount, 1);
	TestEqual(TEXT("Amount entry preserves requested damage"), Listener->LastDamageContext.DamageAmount, 20.0f);
	TestEqual(TEXT("Amount entry reports applied damage"), Listener->LastAppliedDamage, 20.0f);
	TestNull(TEXT("Amount entry defaults instigator to null"), Listener->LastDamageContext.DamageInstigator.Get());
	TestNull(TEXT("Amount entry defaults source to null"), Listener->LastDamageContext.DamageSource.Get());
	TestEqual(TEXT("Amount entry defaults hit location to zero"), Listener->LastDamageContext.DamageHitLocation, FVector::ZeroVector);
	TestTrue(TEXT("Amount entry defaults damage type to none"), Listener->LastDamageContext.DamageType == EDamageType::None);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStructuredDamagePayloadTest,
	"AshenStep.Health.DamageContext.PayloadPreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStructuredDamagePayloadTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);

	AActor* Instigator = GWorld->SpawnActor<AActor>();
	AActor* Source = GWorld->SpawnActor<AActor>();
	FDamageContext DamageContext;
	DamageContext.DamageAmount = 25.0f;
	DamageContext.DamageInstigator = Instigator;
	DamageContext.DamageSource = Source;
	DamageContext.DamageHitLocation = FVector(125.0f, -40.0f, 300.0f);
	DamageContext.DamageType = EDamageType::Fire;

	TestEqual(TEXT("Structured damage reports its applied amount"), Health->ApplyDamage(DamageContext), 25.0f);
	TestEqual(TEXT("Valid damage broadcasts once"), Listener->DamageReceivedCount, 1);
	TestEqual(TEXT("Event preserves requested amount"), Listener->LastDamageContext.DamageAmount, 25.0f);
	TestEqual(TEXT("Event reports actual applied amount"), Listener->LastAppliedDamage, 25.0f);
	TestEqual(TEXT("Event preserves instigator"), Listener->LastDamageContext.DamageInstigator.Get(), Instigator);
	TestEqual(TEXT("Event preserves source"), Listener->LastDamageContext.DamageSource.Get(), Source);
	TestEqual(TEXT("Event preserves world-space hit location"), Listener->LastDamageContext.DamageHitLocation, DamageContext.DamageHitLocation);
	TestTrue(TEXT("Event preserves damage classification"), Listener->LastDamageContext.DamageType == EDamageType::Fire);

	GWorld->DestroyActor(Source);
	GWorld->DestroyActor(Instigator);
	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnvironmentalDamageTest,
	"AshenStep.Health.DamageContext.EnvironmentalDamageAllowsMissingActors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnvironmentalDamageTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);

	FDamageContext DamageContext;
	DamageContext.DamageAmount = 15.0f;
	DamageContext.DamageType = EDamageType::Fire;

	TestEqual(TEXT("Environmental damage remains valid without actor references"), Health->ApplyDamage(DamageContext), 15.0f);
	TestEqual(TEXT("Environmental damage broadcasts once"), Listener->DamageReceivedCount, 1);
	TestNull(TEXT("Missing instigator remains null"), Listener->LastDamageContext.DamageInstigator.Get());
	TestNull(TEXT("Missing source remains null"), Listener->LastDamageContext.DamageSource.Get());

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStructuredDamageRejectionTest,
	"AshenStep.Health.DamageContext.InvalidAndPostDeathDoNotBroadcast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStructuredDamageRejectionTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnHealthChanged.AddDynamic(Listener, &UHealthComponentTestListener::HandleHealthChanged);
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);

	FDamageContext DamageContext;
	DamageContext.DamageAmount = 0.0f;
	TestEqual(TEXT("Zero structured damage is rejected"), Health->ApplyDamage(DamageContext), 0.0f);
	DamageContext.DamageAmount = -10.0f;
	TestEqual(TEXT("Negative structured damage is rejected"), Health->ApplyDamage(DamageContext), 0.0f);
	TestEqual(TEXT("Invalid damage does not broadcast health change"), Listener->HealthChangedCount, 0);
	TestEqual(TEXT("Invalid damage does not broadcast"), Listener->DamageReceivedCount, 0);
	TestEqual(TEXT("Invalid damage does not broadcast death"), Listener->DeathCount, 0);

	DamageContext.DamageAmount = 100.0f;
	Health->ApplyDamage(DamageContext);
	DamageContext.DamageAmount = 10.0f;
	TestEqual(TEXT("Post-death structured damage is rejected"), Health->ApplyDamage(DamageContext), 0.0f);
	TestEqual(TEXT("Only the lethal hit broadcasts health change"), Listener->HealthChangedCount, 1);
	TestEqual(TEXT("Only the lethal hit broadcasts damage"), Listener->DamageReceivedCount, 1);
	TestEqual(TEXT("Only the lethal hit broadcasts death"), Listener->DeathCount, 1);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMultipleStructuredDamageTest,
	"AshenStep.Health.DamageContext.MultipleValidHitsBroadcastOnceEach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultipleStructuredDamageTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnHealthChanged.AddDynamic(Listener, &UHealthComponentTestListener::HandleHealthChanged);
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);

	FDamageContext FirstHit;
	FirstHit.DamageAmount = 10.0f;
	FirstHit.DamageType = EDamageType::Blunt;
	FDamageContext SecondHit;
	SecondHit.DamageAmount = 35.0f;
	SecondHit.DamageType = EDamageType::Slice;

	TestEqual(TEXT("First hit applies once"), Health->ApplyDamage(FirstHit), 10.0f);
	TestEqual(TEXT("Second hit applies once"), Health->ApplyDamage(SecondHit), 35.0f);
	TestEqual(TEXT("Two hits leave expected health"), Health->GetCurrentHealth(), 55.0f);
	TestEqual(TEXT("Each valid hit broadcasts one health change"), Listener->HealthChangedCount, 2);
	TestEqual(TEXT("Each valid hit broadcasts one damage event"), Listener->DamageReceivedCount, 2);
	TestEqual(TEXT("Nonlethal hits do not broadcast death"), Listener->DeathCount, 0);
	TestEqual(TEXT("Latest event carries second requested amount"), Listener->LastDamageContext.DamageAmount, 35.0f);
	TestTrue(TEXT("Latest event carries second damage type"), Listener->LastDamageContext.DamageType == EDamageType::Slice);

	Listener->RemoveFromRoot();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStructuredOverkillAndDeathContextTest,
	"AshenStep.Health.DamageContext.OverkillAndDeathContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStructuredOverkillAndDeathContextTest::RunTest(const FString& Parameters)
{
	AshenStep::Tests::FHealthTestFixture Fixture;
	UHealthComponent* Health = Fixture.GetHealth();
	UHealthComponentTestListener* Listener = NewObject<UHealthComponentTestListener>();
	Listener->AddToRoot();
	Health->OnHealthChanged.AddDynamic(Listener, &UHealthComponentTestListener::HandleHealthChanged);
	Health->OnDamageReceived.AddDynamic(Listener, &UHealthComponentTestListener::HandleDamageReceived);
	Health->OnDeath.AddDynamic(Listener, &UHealthComponentTestListener::HandleDeath);

	FDamageContext DamageContext;
	DamageContext.DamageAmount = 250.0f;
	AActor* Instigator = GWorld->SpawnActor<AActor>();
	AActor* Source = GWorld->SpawnActor<AActor>();
	DamageContext.DamageInstigator = Instigator;
	DamageContext.DamageSource = Source;
	DamageContext.DamageHitLocation = FVector(10.0f, 20.0f, 30.0f);
	DamageContext.DamageType = EDamageType::Slice;

	TestEqual(TEXT("Overkill applies only remaining health"), Health->ApplyDamage(DamageContext), 100.0f);
	TestEqual(TEXT("Event preserves requested overkill damage"), Listener->LastDamageContext.DamageAmount, 250.0f);
	TestEqual(TEXT("Event distinguishes actual overkill damage"), Listener->LastAppliedDamage, 100.0f);
	TestEqual(TEXT("Death broadcasts exactly once"), Listener->DeathCount, 1);
	TestEqual(TEXT("Death preserves lethal hit amount"), Listener->LastDeathContext.DamageAmount, 250.0f);
	TestEqual(TEXT("Death preserves lethal instigator"), Listener->LastDeathContext.DamageInstigator.Get(), Instigator);
	TestEqual(TEXT("Death preserves lethal source"), Listener->LastDeathContext.DamageSource.Get(), Source);
	TestEqual(TEXT("Death preserves lethal hit location"), Listener->LastDeathContext.DamageHitLocation, DamageContext.DamageHitLocation);
	TestTrue(TEXT("Death preserves lethal damage type"), Listener->LastDeathContext.DamageType == EDamageType::Slice);
	TestEqual(TEXT("Health change is first"), Listener->EventOrder[0], FName(TEXT("HealthChanged")));
	TestEqual(TEXT("Damage received is second"), Listener->EventOrder[1], FName(TEXT("DamageReceived")));
	TestEqual(TEXT("Death is third"), Listener->EventOrder[2], FName(TEXT("Death")));

	DamageContext.DamageAmount = 10.0f;
	Health->ApplyDamage(DamageContext);
	TestEqual(TEXT("Post-death damage does not repeat death"), Listener->DeathCount, 1);

	GWorld->DestroyActor(Source);
	GWorld->DestroyActor(Instigator);
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
