// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DKAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOBILEPROJECT_API UDKAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDKAbilitySystemComponent();

	// 만약 Targeting 중이면 Release가 Confirm으로 전환하기
	virtual void AbilityLocalInputReleased(int32 InputID) override;
protected:
	// 오너-클라 전용 플래그(루즈태그): 릴리즈 -> Confirm 전환 스위치
	void MarkAwaitingConfirm(bool bEnable);
};
