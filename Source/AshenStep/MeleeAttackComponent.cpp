// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeAttackComponent.h"
#include "HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

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

	// ...
}

bool UMeleeAttackComponent::RequestMelee()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	UHealthComponent* OwnerHealthComponent = OwnerActor->FindComponentByClass<UHealthComponent>();
	if (!OwnerHealthComponent)
	{
		return false;
	}

	if (OwnerHealthComponent->IsAlive() == false)
	{
		return false;
	}

	USkeletalMeshComponent* OwnerSkeleton = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerSkeleton)
	{
		return false;
	}

	UAnimInstance* OwnerAnim = OwnerSkeleton->GetAnimInstance();
	if (!OwnerAnim)
	{
		return false;
	}

	if (MeleeAttackData.IsValid() == false)
	{
		return false;
	}

	bool bMeleeStatus = MeleeAttackModel.TryStartAttack();

	return bMeleeStatus;
}
