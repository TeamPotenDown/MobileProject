// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SkyeChargeAttack.generated.h"

/**
 * 2초간 차지를 유지하면 자동발사
 * 계속 홀드하고있다면 -> 계속 홀드 스킬이 나가게 설계할 것
 */
UCLASS()
class MOBILEPROJECT_API UGA_SkyeChargeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SkyeChargeAttack();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Animation")
	TObjectPtr<class UAnimMontage> ChargeActionMontage;
	UPROPERTY(EditDefaultsOnly, Category="GAS|Damage")
	TSubclassOf<class UGameplayEffect> ChargeHitEffect;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitDelay> DelayTask;
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitInputRelease> WaitReleaseTask;
	
	UPROPERTY(EditDefaultsOnly)
	float StartTime = 0.f; // 차지 시작시간
	UPROPERTY(EditDefaultsOnly)
	float HoldThreshold = 1.0f; // 차징시간
	UPROPERTY(EditDefaultsOnly)
	bool bHasFired = false; // 자동발사 여부

protected:
	UFUNCTION()
	void BeginCharging();
	UFUNCTION()
	void FireChargedSkill();
	UFUNCTION()
	void OnInputRelease(float Time);
	UFUNCTION()
	void EndChargeAttack(bool bWasCancelled);

	UFUNCTION()
	void Server_ApplyChargeDamage();
	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass) const;
};
