// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_DKAttack.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_DKAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_DKAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	/** 최대 콤보 횟수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	int32 MaxComboCount = 4;

	/** 콤보용 애니메이션 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<class UAnimMontage> ComboMontage = nullptr;

private:
	int32 CurrentCombo = 0;
	bool bCanAcceptInput = false;

	/* AbilityTasks */
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

	UPROPERTY()
	TSubclassOf<class UGameplayEffect> WindowEffectClass = nullptr;

	/** 콜백  */
	UFUNCTION()
	void HandleMontageCompleted();
};
