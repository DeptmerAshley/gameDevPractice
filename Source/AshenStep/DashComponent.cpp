// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"

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

	if (!IsValid(CachedCharacter) || !IsValid(CachedMovementComponent))
	{
		return;
	}

	DashAbilityModel.AdvanceTime(DeltaTime);

	if (DashAbilityModel.GetState() == EDashState::Dashing)
	{
		//Determine frame displacement
		// Attempt collision-safe movement
	}
	// Advance the models time/state
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

	return DashAbilityModel.TryStartDash(
		MovementInput,
		ForwardDirection,
		RightDirection,
		bIsAirborne);
}
