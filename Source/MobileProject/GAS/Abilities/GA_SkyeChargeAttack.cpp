// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SkyeChargeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/Character.h"

UGA_SkyeChargeAttack::UGA_SkyeChargeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;
	
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("InputTag.PrimaryAttack.Normal"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attack.Charging"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attack.Combo"));
}

void UGA_SkyeChargeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bHasFired = false;
	StartTime = GetWorld()->GetTimeSeconds();

	BeginCharging();

	// 대기 후 자동발사
	DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, HoldThreshold);
	DelayTask->OnFinish.AddDynamic(this, &UGA_SkyeChargeAttack::FireChargedSkill);
	DelayTask->ReadyForActivation();

	// 조기 해제 판단
	WaitReleaseTask  = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_SkyeChargeAttack::OnInputRelease);
	WaitReleaseTask->ReadyForActivation();
}

void UGA_SkyeChargeAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	EndChargeAttack(true);
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_SkyeChargeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkyeChargeAttack::InputPressed(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bHasFired) return;

	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UGA_SkyeChargeAttack::BeginCharging()
{
	// TODO: 이동막기, 차지FX 등
}

void UGA_SkyeChargeAttack::FireChargedSkill()
{
	if (bHasFired) return;
	bHasFired = true;

	if (HasAuthority(&CurrentActivationInfo))
	{
		Server_ApplyChargeDamage();
	}
}

void UGA_SkyeChargeAttack::OnInputRelease(float Time)
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
}

void UGA_SkyeChargeAttack::EndChargeAttack(bool bWasCancelled)
{
	// 태스크 정리
	if (DelayTask) { DelayTask->EndTask(); DelayTask = nullptr; }
	if (WaitReleaseTask) { WaitReleaseTask->EndTask(); WaitReleaseTask = nullptr; }

	// TODO: 이동 여기서 복구
}

void UGA_SkyeChargeAttack::Server_ApplyChargeDamage()
{
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	ACharacter* Owner = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
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

	// 이번 타의 GE (인덱스: 0,1,2)
	TSubclassOf<UGameplayEffect> Effect = ChargeHitEffect;
	
	for (const FHitResult& Hit : HitResults)
	{
		ApplyDamageToTarget(Hit.GetActor(), Effect);
	}
}

void UGA_SkyeChargeAttack::ApplyDamageToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass) const
{
	if (!TargetActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass, 1.f);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
