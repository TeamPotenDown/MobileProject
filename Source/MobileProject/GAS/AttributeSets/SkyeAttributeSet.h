// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SkyeAttributeSet.generated.h"

/**
AttributeSet은 속석정의만 담당, 실제 값은 GameplayEffect로 초기화
→ 게임 시작 시, 캐릭터에게 GE_InitStats라는 초기화 GE를 적용
 */

UCLASS()
class MOBILEPROJECT_API USkyeAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// ATTRIBUTE_ACCESSORS가 ATTRIBUTE_ACCESSORS_BASIC으로 변경됨
	// 이 매크로를 사용했다면 자동으로 초기화 함수가 생성됨
	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing = OnRep_CurrentHealth)
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS_BASIC(USkyeAttributeSet, CurrentHealth)

	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(USkyeAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category="Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(USkyeAttributeSet, Damage)
	
	UFUNCTION()
	void OnRep_CurrentHealth(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;
};
