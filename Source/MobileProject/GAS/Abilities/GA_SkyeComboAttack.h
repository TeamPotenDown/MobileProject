// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SkyeComboAttack.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_SkyeComboAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SkyeComboAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Animation")
	TObjectPtr<class UAnimMontage> ComboActionMontage;
	UPROPERTY(EditDefaultsOnly, Category="GAS")
	int32 MaxComboCount = 3;
	UPROPERTY(EditDefaultsOnly, Category="GAS|Damage")
	TArray<TSubclassOf<UGameplayEffect>> ComboHitEffects;
	
	UPROPERTY(EditDefaultsOnly)
	int32 CurrentComboIndex = 0;
	UPROPERTY(EditDefaultsOnly)
	bool bInputWindowOpen = false;
	UPROPERTY(EditDefaultsOnly)
	bool bInputNext = false;
	UPROPERTY(EditDefaultsOnly)
	bool bSectionEnding = false;
	
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> HitEventTask;
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> InputOpenTask;
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> InputCloseTask;

protected:
	UFUNCTION()
	void StartFirstOrNextSection();
	UFUNCTION()
	void BindSectionEvents();
	UFUNCTION()
	void ClearSectionEvents();
	UFUNCTION()
	void OnSectionEnded();

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnInputOpenReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnInputCloseReceived(FGameplayEventData Payload);

	UFUNCTION()
	void Server_ApplyComboDamage();
	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass) const;
};


