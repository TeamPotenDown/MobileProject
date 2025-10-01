// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MobileProject/Character/Player/DKCharacter/Input/EAbilityInputID.h"
#include "GA_DK_GameplayAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_DK_GameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	EAbilityInputID GetInputID() const { return InputID; }

protected:
	EAbilityInputID InputID = EAbilityInputID::None;
};
