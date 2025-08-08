// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_DKAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagContainer.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "MobileProject/Utils/LogUtils.h"

DEFINE_LOG_CATEGORY(LogMP);

UGA_DKAttack::UGA_DKAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
		bReplicateInputDirectly = true;

	AbilityTags.AddTag(MP_TAG_DK_PRIMARY_ATTACK);
}

void UGA_DKAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentCombo = 1;
	bCanAcceptInput = false;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ComboMontage, 1.0f, FName(TEXT("Combo1")));
	MontageTask->OnCompleted.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->ReadyForActivation();
}

void UGA_DKAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();
	if (ASC)
	{
		FGameplayTag TagToRemove = MP_TAG_DK_STATE_COMBO_WINDOW_OPEN;;
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(TagToRemove);
		
		ASC->RemoveActiveEffectsWithGrantedTags(TagContainer);
	} 
		
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DKAttack::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Ensured();
	if (!ASC || !ASC->HasMatchingGameplayTag(MP_TAG_DK_STATE_COMBO_WINDOW_OPEN))
	{
		return;
	}

	if (CurrentCombo >= MaxComboCount)
	{
		return;
	}

	{
		FScopedPredictionWindow PredictionWindow(ASC, true);
		
		FGameplayTag TagToRemove = MP_TAG_DK_STATE_COMBO_WINDOW_OPEN;;
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(TagToRemove);
		
		ASC->RemoveActiveEffectsWithGrantedTags(TagContainer);
	}
	
	++CurrentCombo;

	const FName MontageSectionName = *FString::Printf(TEXT("Combo%d"), CurrentCombo);
	MontageJumpToSection(MontageSectionName);
}

void UGA_DKAttack::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}