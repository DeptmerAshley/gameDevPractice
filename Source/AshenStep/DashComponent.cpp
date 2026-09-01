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

	AActor* Actor = GetOwner();
	if (!Actor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}
}


// Called every frame
void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DashAbilityModel.AdvanceTime(DeltaTime);
}

bool UDashComponent::RequestDash(const FVector2D& MovementInput)
{
	AActor* Actor = GetOwner();
	if (!Actor)
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	const bool bIsAirborne = MovementComponent->IsFalling();
	
	const FRotator ControlRotation = Character->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	return DashAbilityModel.TryStartDash(
		MovementInput,
		ForwardDirection,
		RightDirection,
		bIsAirborne);
}
