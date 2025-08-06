// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SkyeNormalAttack.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

UGA_SkyeNormalAttack::UGA_SkyeNormalAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_SkyeNormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!FirstHitMontage || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();

	// 노티파이 바인딩
	if (AnimInstance)
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_SkyeNormalAttack::OnMontageNotifyBegin);
	}

	AnimInstance->Montage_Play(FirstHitMontage);
}

void UGA_SkyeNormalAttack::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName == "FirstHit")
	{
		ApplyFirstHitEffect();
	}
}

void UGA_SkyeNormalAttack::ApplyFirstHitEffect()
{
	if (!GE_FirstHit) return;
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor) return;

	
}
