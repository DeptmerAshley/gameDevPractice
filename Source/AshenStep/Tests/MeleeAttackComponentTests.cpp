#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Keep the character header self-contained: do not hide missing forward declarations
// by including MeleeAttackComponent.h before it.
#include "AshenStepCharacter.h"
#include "Animation/AnimMontage.h"
#include "Combat/MeleeAttackData.h"
#include "Combat/MeleeAttackModel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"
#include "HealthComponent.h"
#include "MeleeAttackComponent.h"
#include "Tests/MeleeAttackTestCharacter.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

// TDD contract and asset-dependent follow-up cases: docs/day6-phase3-tdd.md.
// RequestMelee(), GetState(), and CanRegisterHits() are expected component APIs,
// not test-side implementations of the combat rules.
namespace AshenStep::MeleeComponentTests
{
constexpr EAutomationTestFlags TestFlags =
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

class FTestWorld
{
public:
	FTestWorld()
	{
		if (GEngine)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World)
			{
				GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
				World->InitializeActorsForPlay(FURL());
			}
		}
	}

	~FTestWorld()
	{
		if (World)
		{
			World->DestroyWorld(false);
			GEngine->DestroyWorldContext(World);
		}
	}

	AAshenStepCharacter* SpawnCharacter() const
	{
		if (!World)
		{
			return nullptr;
		}
		FActorSpawnParameters Parameters;
		Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AMeleeAttackTestCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Parameters);
	}

private:
	UWorld* World = nullptr;
};

// Configure through the same reflected struct used by Blueprint defaults.
// No public setter or access to the component's private state is required.
// Find by type, allowing Martin to choose the configuration property's name.
FMeleeAttackData* FindAttackData(UMeleeAttackComponent* Component)
{
	for (TFieldIterator<FStructProperty> Property(Component->GetClass()); Property; ++Property)
	{
		if (Property->Struct == FMeleeAttackData::StaticStruct() && Property->HasAnyPropertyFlags(CPF_Edit))
		{
			return Property->ContainerPtrToValuePtr<FMeleeAttackData>(Component);
		}
	}
	return nullptr;
}

bool AssertReadyWithoutHits(FAutomationTestBase& Test, const UMeleeAttackComponent& Component)
{
	const bool bReady = Test.TestEqual(TEXT("Component remains ready"), Component.GetState(), EMeleeState::Ready);
	const bool bNoHits = Test.TestFalse(TEXT("Component cannot register hits"), Component.CanRegisterHits());
	return bReady && bNoHits;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeComponentOwnershipTest,
	"AshenStep.MeleeAttack.Phase3.Component.CharacterOwnership",
	AshenStep::MeleeComponentTests::TestFlags)

bool FMeleeComponentOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace AshenStep::MeleeComponentTests;
	FTestWorld Fixture;
	AAshenStepCharacter* First = Fixture.SpawnCharacter();
	AAshenStepCharacter* Second = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("First character spawns"), First) ||
		!TestNotNull(TEXT("Second character spawns"), Second))
	{
		return false;
	}

	UMeleeAttackComponent* FirstAttack = First->GetMeleeAttackComponent();
	UMeleeAttackComponent* SecondAttack = Second->GetMeleeAttackComponent();
	if (!TestNotNull(TEXT("First character owns melee"), FirstAttack) ||
		!TestNotNull(TEXT("Second character owns melee"), SecondAttack))
	{
		return false;
	}
	TestTrue(TEXT("Characters do not share a combat component"), FirstAttack != SecondAttack);
	TestTrue(TEXT("Component owner is its character"), FirstAttack->GetOwner() == First);
	TestEqual(TEXT("Getter returns the attached component"), FirstAttack, First->FindComponentByClass<UMeleeAttackComponent>());
	TArray<UMeleeAttackComponent*> Components;
	First->GetComponents<UMeleeAttackComponent>(Components);
	TestEqual(TEXT("Character has exactly one melee component"), Components.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeComponentSafeDefaultsTest,
	"AshenStep.MeleeAttack.Phase3.Component.SafeDefaults",
	AshenStep::MeleeComponentTests::TestFlags)

bool FMeleeComponentSafeDefaultsTest::RunTest(const FString& Parameters)
{
	using namespace AshenStep::MeleeComponentTests;
	FTestWorld Fixture;
	AAshenStepCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Character spawns"), Character)) { return false; }
	UMeleeAttackComponent* Attack = Character->GetMeleeAttackComponent();
	if (!TestNotNull(TEXT("Character has melee"), Attack)) { return false; }
	FMeleeAttackData* Data = FindAttackData(Attack);
	if (!TestNotNull(TEXT("Component exposes editable attack configuration"), Data)) { return false; }
	TestEqual(TEXT("Unconfigured component has harmless damage"), Data->DamageAmount, 0.0f);
	TestFalse(TEXT("Unconfigured component is not attack-ready configuration"), Data->IsValid());
	Character->DispatchBeginPlay();
	AssertReadyWithoutHits(*this, *Attack);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeComponentUnconfiguredRequestTest,
	"AshenStep.MeleeAttack.Phase3.Component.UnconfiguredRequestsAreHarmless",
	AshenStep::MeleeComponentTests::TestFlags)

bool FMeleeComponentUnconfiguredRequestTest::RunTest(const FString& Parameters)
{
	using namespace AshenStep::MeleeComponentTests;
	FTestWorld Fixture;
	AAshenStepCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Character spawns"), Character)) { return false; }
	UMeleeAttackComponent* Attack = Character->GetMeleeAttackComponent();
	UHealthComponent* Health = Character->GetHealthComponent();
	if (!TestNotNull(TEXT("Character has melee"), Attack) || !TestNotNull(TEXT("Character has health"), Health)) { return false; }
	Character->DispatchBeginPlay();
	if (!TestTrue(TEXT("Rejection fixture is alive"), Health->IsAlive())) { return false; }
	const float HealthBefore = Health->GetCurrentHealth();
	for (int32 Press = 0; Press < 5; ++Press)
	{
		TestFalse(TEXT("An unconfigured attack request is rejected"), Attack->RequestMelee());
		AssertReadyWithoutHits(*this, *Attack);
	}
	TestEqual(TEXT("Rejected requests do not change attacker health"), Health->GetCurrentHealth(), HealthBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeComponentMissingAnimationTest,
	"AshenStep.MeleeAttack.Phase3.Component.MissingAnimationDoesNotLatchAttack",
	AshenStep::MeleeComponentTests::TestFlags)

bool FMeleeComponentMissingAnimationTest::RunTest(const FString& Parameters)
{
	using namespace AshenStep::MeleeComponentTests;
	FTestWorld Fixture;
	AAshenStepCharacter* Character = Fixture.SpawnCharacter();
	if (!TestNotNull(TEXT("Character spawns"), Character)) { return false; }
	UMeleeAttackComponent* Attack = Character->GetMeleeAttackComponent();
	if (!TestNotNull(TEXT("Character has melee"), Attack)) { return false; }
	FMeleeAttackData* Data = FindAttackData(Attack);
	if (!TestNotNull(TEXT("Component has editable attack configuration"), Data)) { return false; }
	// This is deliberately NOT a playable montage. It satisfies configuration
	// validation so a bare C++ character with no anim instance must reject safely.
	Data->AttackMontage = NewObject<UAnimMontage>(Attack);
	Data->DamageAmount = 25.0f;
	Data->MontagePlayRate = 1.0f;
	Data->TraceRadius = 12.0f;
	Data->TraceStartSocket = TEXT("weapon_base");
	Data->TraceEndSocket = TEXT("weapon_tip");
	if (!TestTrue(TEXT("Configuration fields pass structural validation"), Data->IsValid())) { return false; }
	Character->DispatchBeginPlay();
	if (!TestNotNull(TEXT("Character has health"), Character->GetHealthComponent()) ||
		!TestTrue(TEXT("Character is alive"), Character->GetHealthComponent()->IsAlive()) ||
		!TestNotNull(TEXT("Character has a mesh component"), Character->GetMesh())) { return false; }
	TestNull(TEXT("Bare character has no animation instance"), Character->GetMesh()->GetAnimInstance());
	for (int32 Press = 0; Press < 3; ++Press)
	{
		TestFalse(TEXT("Unavailable animation rejects the request"), Attack->RequestMelee());
		AssertReadyWithoutHits(*this, *Attack);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMeleeComponentNoOwnerTest,
	"AshenStep.MeleeAttack.Phase3.Component.MissingOwnerRejectsSafely",
	AshenStep::MeleeComponentTests::TestFlags)

bool FMeleeComponentNoOwnerTest::RunTest(const FString& Parameters)
{
	using namespace AshenStep::MeleeComponentTests;
	TStrongObjectPtr<UMeleeAttackComponent> Attack(NewObject<UMeleeAttackComponent>());
	TestNull(TEXT("Fixture deliberately has no owner"), Attack->GetOwner());
	TestFalse(TEXT("Ownerless request is rejected without dereferencing an actor"), Attack->RequestMelee());
	AssertReadyWithoutHits(*this, *Attack);
	return true;
}

#endif
