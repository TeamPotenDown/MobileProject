#include "GA_DK_PassiveTargetingAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"

UGA_DK_PassiveTargetingAbility::UGA_DK_PassiveTargetingAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	InputID = EAbilityInputID::None; // Passive Targeting Ability
}

void UGA_DK_PassiveTargetingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Passive TA를 실행 시켜야 함.
	// 하지만 이를 기다리는 AbilityTask가 필요함.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;		
	}

	ActivateTargetActor();
}

void UGA_DK_PassiveTargetingAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ContinuousWaitTargetDataTask)
	{
		ContinuousWaitTargetDataTask->EndTask();
		ContinuousWaitTargetDataTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DK_PassiveTargetingAbility::ActivateTargetActor()
{
	ContinuousWaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(
		this,
		NAME_None,
		EGameplayTargetingConfirmation::CustomMulti,
		TargetActorClass
	);

	ContinuousWaitTargetDataTask->ValidData.AddDynamic(this, &UGA_DK_PassiveTargetingAbility::UGA_DK_PassiveTargetingAbility::OnTargetDataReady);

	AGameplayAbilityTargetActor* SpawnedActor = nullptr;
	if (ContinuousWaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, SpawnedActor))
	{
		// 여기서 커스텀 세팅도 가능함.
		ContinuousWaitTargetDataTask->FinishSpawningActor(this, SpawnedActor);
	}
	ContinuousWaitTargetDataTask->ReadyForActivation();
}

void UGA_DK_PassiveTargetingAbility::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return ;
	}

	FGameplayEventData EventData;
	EventData.TargetData = Data;
	EventData.Instigator = AvatarActor;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		AvatarActor,
		MP_TAG_DK_EVENT_NOTIFY_PASSIVE_TARGETING_READY,
		EventData
	);
}

