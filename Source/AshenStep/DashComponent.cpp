// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UDashComponent::UDashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	FDashConfig DashConfig;
	DashConfig.Distance = DashDistance;
	DashConfig.Duration = DashDuration;
	DashConfig.Cooldown = DashCooldown;
	DashConfig.bAllowAirDash = bAllowAirDash;

	DashAbilityModel = FDashAbilityModel(DashConfig);
	
	//Load an Actor cast as a character into the cached character slot
	CachedCharacter = Cast<ACharacter>(GetOwner());

	if (!CachedCharacter)
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	//Load the movement component from the cached character into the cached movement component slot
	CachedMovementComponent = CachedCharacter->GetCharacterMovement();

	if (!CachedMovementComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}
}


// Called every frame
void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bCollision = false;

	if (!IsValid(CachedCharacter) || !IsValid(CachedMovementComponent))
	{
		return;
	}

	if (DashAbilityModel.GetState() == EDashState::Dashing)
	{
		//Determine frame displacement
		// Attempt collision-safe movement
		const FVector DashDirection = DashAbilityModel.GetDashDirection();

		// const float DashSpeed = DashAbilityModel.GetDashSpeed();
		const float DashSpeed = ActiveDashSpeed;

		const FVector FrameDisplacement = DashDirection * DashSpeed * DeltaTime;

		FHitResult Hit;

		CachedMovementComponent->SafeMoveUpdatedComponent(
			FrameDisplacement,
			CachedCharacter->GetActorQuat(),
			true,
			Hit
		);

		if (Hit.IsValidBlockingHit())
		{
			bCollision = true;
			DashAbilityModel.EndDashEarly();
		}
	}

	// Advance the models time/state
	DashAbilityModel.AdvanceTime(DeltaTime);
}

bool UDashComponent::RequestDash(const FVector2D& MovementInput)
{
	if (!IsValid(CachedCharacter) || !IsValid(CachedMovementComponent))
	{
		return false;
	}

	const bool bIsAirborne = CachedMovementComponent->IsFalling();
	
	const FRotator ControlRotation = CachedCharacter->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const bool bDashStarted = DashAbilityModel.TryStartDash(MovementInput, ForwardDirection, RightDirection, bIsAirborne);
	if (!bDashStarted)
	{
		return false;
	}

	FVector CurrentVelocity = CachedMovementComponent->Velocity;
	FVector HorizontalVelocity = CurrentVelocity;
	HorizontalVelocity.Z = 0.0f;
	const float BaseDashSpeed = DashAbilityModel.GetDashSpeed();
	const FVector& DashDirection = DashAbilityModel.GetDashDirection();

	const float ForwardMomentum = FVector::DotProduct(HorizontalVelocity, DashDirection);
	const float UnclampedMomentumBonus = FMath::Max(ForwardMomentum, 0.0f) * MomentumContribution;

	const float MomentumBonus = FMath::Clamp(UnclampedMomentumBonus, 0.0f, MaxMomentumBonus);

	ActiveDashSpeed = BaseDashSpeed + MomentumBonus;
	CachedMovementComponent->StopMovementImmediately();

	return true;
}

bool UDashComponent::IsDashing() const
{
	if ((DashAbilityModel.GetState()) == EDashState::Dashing)
	{
		return true;
	}
	return false;
}