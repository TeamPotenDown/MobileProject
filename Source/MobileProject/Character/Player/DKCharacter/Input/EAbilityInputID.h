#pragma once

#include "EAbilityInputID.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None    UMETA(DisplayName="None"),
	Confirm UMETA(DisplayName="Confirm"),
	Cancel  UMETA(DisplayName="Cancel"),

	Ability1 UMETA(DisplayName="Ability1"), // 기본공격
	Ability2 UMETA(DisplayName="Ability2"), // 특수 공격
	Ability3 UMETA(DisplayName="Ability3"), // 궁극기
	Sprint   UMETA(DisplayName="Sprint"),
	Jump     UMETA(DisplayName="Jump"),
};
