#pragma once
#include "ESkyeAbilityEnum.generated.h"

UENUM(BlueprintType)
enum class ESkyeAbilityEnum : uint8
{
	None           UMETA(DisplayName = "None"),
	ComboAttack    UMETA(DisplayName = "ComboAttack"), // 기본 공격 (탭)
	ChargeAttack   UMETA(DisplayName = "ChargeAttack"), // 기본 공격 (홀드)
	WarpAttack     UMETA(DisplayName = "WarpAttack"), // 특수 공격 (적중시 뒤로 순간이동)
	UltimateAttack UMETA(DisplayName = "UltimateAttack"), // 궁극기
	Dash           UMETA(DisplayName = "Dash"), // 대쉬
};