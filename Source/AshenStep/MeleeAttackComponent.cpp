// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttackComponent.h"
#include "AshenStep.h"
#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

namespace
{
	const TCHAR* MeleeStateName(EMeleeState State)
	{
		switch (State)
		{
		case EMeleeState::Ready: return TEXT("Ready");
		case EMeleeState::WindUp: return TEXT("WindUp");
		case EMeleeState::Active: return TEXT("Active");
		case EMeleeState::Recovery: return TEXT("Recovery");
		default: return TEXT("Unknown");
		}
	}

	// Event-only diagnostics: never log from TickComponent or state queries.
	void LogMeleeTransition(const AActor* Owner, const TCHAR* Event, bool bAccepted,
		EMeleeState Before, const FMeleeAttackModel& Model)
	{
		UE_LOG(LogAshenStep, Log,
			TEXT("[Melee] %s | %s | %s | %s -> %s | HitsAllowed=%s"),
			*GetNameSafe(Owner), Event, bAccepted ? TEXT("accepted") : TEXT("rejected"),
			MeleeStateName(Before), MeleeStateName(Model.GetState()),
			Model.CanRegisterHits() ? TEXT("true") : TEXT("false"));
	}
}

// Sets default values for this component's properties
UMeleeAttackComponent::UMeleeAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMeleeAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CanRegisterHits())
	{
		return;
	}
}

bool UMeleeAttackComponent::RequestMelee()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] RequestMelee rejected: missing owner"));
		return false;
	}

	UHealthComponent* OwnerHealthComponent = OwnerActor->FindComponentByClass<UHealthComponent>();
	if (!OwnerHealthComponent)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: missing health component"), *GetNameSafe(OwnerActor));
		return false;
	}

	if (OwnerHealthComponent->IsAlive() == false)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: owner is dead"), *GetNameSafe(OwnerActor));
		return false;
	}

	USkeletalMeshComponent* OwnerSkeleton = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerSkeleton)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: missing skeletal mesh"), *GetNameSafe(OwnerActor));
		return false;
	}

	if (OwnerSkeleton->DoesSocketExist(MeleeAttackData.TraceStartSocket) == false || OwnerSkeleton->DoesSocketExist(MeleeAttackData.TraceEndSocket) == false)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: missing skeletal socket"), *GetNameSafe(OwnerActor));
		return false;
	}

	UAnimInstance* OwnerAnim = OwnerSkeleton->GetAnimInstance();
	if (!OwnerAnim)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: missing animation instance"), *GetNameSafe(OwnerActor));
		return false;
	}

	if (MeleeAttackData.IsValid() == false)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | RequestMelee rejected: invalid attack configuration"), *GetNameSafe(OwnerActor));
		return false;
	}

	const EMeleeState StateBeforeStart = MeleeAttackModel.GetState();
	bool bMeleeStatus = MeleeAttackModel.TryStartAttack();
	LogMeleeTransition(OwnerActor, TEXT("TryStartAttack"), bMeleeStatus, StateBeforeStart, MeleeAttackModel);

	if (!bMeleeStatus)
	{
		return false;
	}

	float MontageLength = OwnerAnim->Montage_Play(MeleeAttackData.AttackMontage, MeleeAttackData.MontagePlayRate);

	if (MontageLength == 0.0f)
	{
		UE_LOG(LogAshenStep, Warning, TEXT("[Melee] %s | Montage_Play failed | Montage=%s | PlayRate=%.3f"),
			*GetNameSafe(OwnerActor), *GetNameSafe(MeleeAttackData.AttackMontage.Get()), MeleeAttackData.MontagePlayRate);
		const EMeleeState StateBeforeReset = MeleeAttackModel.GetState();
		MeleeAttackModel.Interrupt();
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | Playback failure cleanup | %s -> %s"),
			*GetNameSafe(OwnerActor), MeleeStateName(StateBeforeReset), MeleeStateName(MeleeAttackModel.GetState()));
		return false;
	}

	FOnMontageEnded MontageEnded;
	MontageEnded.BindUObject(this, &UMeleeAttackComponent::OnAttackMontageEnded);

	OwnerAnim->Montage_SetEndDelegate(MontageEnded, MeleeAttackData.AttackMontage);
	UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | Montage started | Montage=%s | Length=%.3f | PlayRate=%.3f"),
		*GetNameSafe(OwnerActor), *GetNameSafe(MeleeAttackData.AttackMontage.Get()), MontageLength, MeleeAttackData.MontagePlayRate);
	return bMeleeStatus;
}

EMeleeState UMeleeAttackComponent::GetState() const
{
	return MeleeAttackModel.GetState();
}

bool UMeleeAttackComponent::CanRegisterHits() const
{
	if (GetState() == EMeleeState::Active)
	{
		return true;
	}
	return false;
}

void UMeleeAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	if (Montage != MeleeAttackData.AttackMontage)
	{
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | MontageEnded ignored: unrelated montage %s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(Montage));
		return;
	}

	const EMeleeState StateBeforeCleanup = MeleeAttackModel.GetState();
	if (bInterrupted)
	{
		MeleeAttackModel.Interrupt();
		UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | MontageEnded | Montage=%s | Interrupted=true | Cleanup=Interrupt | %s -> %s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(Montage), MeleeStateName(StateBeforeCleanup), MeleeStateName(MeleeAttackModel.GetState()));
		return;
	}

	if (MeleeAttackModel.GetState() == EMeleeState::Recovery)
	{
		MeleeAttackModel.TryEndRecovery();
	}
	else
	{
		MeleeAttackModel.Interrupt();
	}
	UE_LOG(LogAshenStep, Log, TEXT("[Melee] %s | MontageEnded | Montage=%s | Interrupted=false | Cleanup=%s | %s -> %s"),
		*GetNameSafe(GetOwner()), *GetNameSafe(Montage),
		StateBeforeCleanup == EMeleeState::Recovery ? TEXT("TryEndRecovery") : TEXT("FallbackInterrupt"),
		MeleeStateName(StateBeforeCleanup), MeleeStateName(MeleeAttackModel.GetState()));
}

bool UMeleeAttackComponent::BeginAttackWindow()
{
	const EMeleeState StateBefore = MeleeAttackModel.GetState();
	const bool bAccepted = MeleeAttackModel.TryAttack();
	LogMeleeTransition(GetOwner(), TEXT("BeginAttackWindow"), bAccepted, StateBefore, MeleeAttackModel);
	return bAccepted;
}

bool UMeleeAttackComponent::EndAttackWindow()
{
	const EMeleeState StateBefore = MeleeAttackModel.GetState();
	const bool bAccepted = MeleeAttackModel.TryEndAttack();
	LogMeleeTransition(GetOwner(), TEXT("EndAttackWindow"), bAccepted, StateBefore, MeleeAttackModel);
	return bAccepted;
}
