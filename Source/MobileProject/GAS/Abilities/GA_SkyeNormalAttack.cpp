// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SkyeNormalAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities//Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "DrawDebugHelpers.h"
#include "MobileProject/Character/Player/Skye/SkyeCharacter.h"

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
	(this, TEXT("PlayAttack"), SkyeCharacter->GetComboActionMontage());

	PlayAttackTask->OnCompleted.AddDynamic(this, &UGA_SkyeNormalAttack::OnCompletedCallback);
	PlayAttackTask->OnInterrupted.AddDynamic(this, &UGA_SkyeNormalAttack::OnInterruptedCallback);
	PlayAttackTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.SkyeNormalHit"));

	WaitEvent->EventReceived.AddDynamic(this, &UGA_SkyeNormalAttack::OnNormalHit);
	WaitEvent->ReadyForActivation();
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

void UGA_SkyeNormalAttack::OnNormalHit(FGameplayEventData Payload)
{
	AActor* Owner = GetAvatarActorFromActorInfo();
	if (!Owner) return;

	FVector StartLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector()*50.f;
	FVector EndLocation = StartLocation + Owner->GetActorForwardVector()*100.f;

	TArray<FHitResult> HitResults;
	FCollisionShape Box = FCollisionShape::MakeBox(FVector(50.f));
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, StartLocation, EndLocation, FQuat::Identity,
		ECC_Visibility, Box, Params);
	
	DrawDebugBox(GetWorld(), EndLocation, Box.GetExtent(), FQuat::Identity, FColor::Red, false, 1.0f);

	for (const FHitResult& Hit : HitResults)
	{
		ApplyDamageToTarget(Hit.GetActor());
	}
}

void UGA_SkyeNormalAttack::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!TargetActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
