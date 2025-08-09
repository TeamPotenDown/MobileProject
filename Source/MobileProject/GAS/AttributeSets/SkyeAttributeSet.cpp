// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyeAttributeSet.h"

#include "Net/UnrealNetwork.h"

void USkyeAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*REPNOTIFY_Always는 로컬 값이 서버에서 내려오는 값과 이미 동일한 경우에도 OnRep 함수가 트리거되도록 설정(예측으로 인해 발생).
	 *기본적으로 로컬 값이 서버에서 내려오는 값과 동일하면 OnRep 함수는 트리거되지 않음.*/
	DOREPLIFETIME_CONDITION_NOTIFY(USkyeAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USkyeAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}
	

void USkyeAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void USkyeAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USkyeAttributeSet, CurrentHealth, OldValue);
}

void USkyeAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USkyeAttributeSet, MaxHealth, OldValue);
}
