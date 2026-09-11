#pragma once

#include "AshenStepCharacter.h"
#include "MeleeAttackTestCharacter.generated.h"

// Concrete fixture: AAshenStepCharacter is intentionally abstract and cannot
// be spawned directly. Inherit its real components without replacing behavior.
UCLASS(NotBlueprintable, Transient)
class AMeleeAttackTestCharacter : public AAshenStepCharacter
{
	GENERATED_BODY()
};
