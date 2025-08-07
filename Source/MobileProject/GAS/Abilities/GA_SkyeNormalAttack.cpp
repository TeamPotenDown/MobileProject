// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SkyeNormalAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities//Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MobileProject/Character/Player/SkyeCharacter.h"

UGA_SkyeNormalAttack::UGA_SkyeNormalAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("InputTag.PrimaryAttack.Normal"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.State.Attacking"));
}

void UGA_SkyeNormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ASkyeCharacter* SkyeCharacter = CastChecked<ASkyeCharacter>(ActorInfo->AvatarActor.Get());
	UAbilityTask_PlayMontageAndWait* PlayAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy
	(this, TEXT("PLayAttack"), SkyeCharacter->GetComboActionMontage());

	PlayAttackTask->OnCompleted.AddDynamic(this, &UGA_SkyeNormalAttack::OnCompletedCallback);
	PlayAttackTask->OnInterrupted.AddDynamic(this, &UGA_SkyeNormalAttack::OnInterruptedCallback);
	PlayAttackTask->ReadyForActivation();
}

void UGA_SkyeNormalAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_SkyeNormalAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkyeNormalAttack::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UE_LOG(LogTemp, Display, TEXT("InputPressed"));
}

void UGA_SkyeNormalAttack::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UGA_SkyeNormalAttack::OnCompletedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UGA_SkyeNormalAttack::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
