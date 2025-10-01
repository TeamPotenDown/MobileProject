#include "GA_DK_PrimaryAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagContainer.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "MobileProject/Utils/LogUtils.h"
#include "MobileProject/GAS/AbilityTasks/AbilityTask_ContinuousDetectInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

DEFINE_LOG_CATEGORY(LogMP);

UGA_DK_PrimaryAbility::UGA_DK_PrimaryAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;

	InputID = EAbilityInputID::Ability1; // 기본공격
}

void UGA_DK_PrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	ActivateComboAbility();
	
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ComboMontage, 1.0f, FName(TEXT("Combo1")));
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_DK_PrimaryAbility::HandleMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_DK_PrimaryAbility::HandleMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_DK_PrimaryAbility::HandleMontageCompleted);
		MontageTask->ReadyForActivation();
	}

	InputPressTask = UAbilityTask_ContinuousDetectInputPress::WaitInputPress(this, false);
	if (InputPressTask)
	{
		InputPressTask->OnInputPressedContinuous.AddDynamic(this, &UGA_DK_PrimaryAbility::OnPressed);
		InputPressTask->ReadyForActivation();
	}
}

void UGA_DK_PrimaryAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	if (InputPressTask)
	{
		InputPressTask->EndTask();
		InputPressTask = nullptr;
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

void UGA_DK_PrimaryAbility::OnPressed()
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
		FGameplayTag TagToRemove = MP_TAG_DK_STATE_COMBO_WINDOW_OPEN;;
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(TagToRemove);
		
		ASC->RemoveActiveEffectsWithGrantedTags(TagContainer);
	}
	
	++CurrentCombo;

	const FName MontageSectionName = *FString::Printf(TEXT("Combo%d"), CurrentCombo);
	MontageJumpToSection(MontageSectionName);

	ActivateComboAbility();
}

void UGA_DK_PrimaryAbility::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_DK_PrimaryAbility::ActivateComboAbility()
{
	if (!ComboEventTags.Contains(CurrentCombo))
	{
		return ;
	}
	
	FGameplayTag ComboTag = ComboEventTags[CurrentCombo];

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetAvatarActorFromActorInfo(), ComboTag, FGameplayEventData()
	);
}
