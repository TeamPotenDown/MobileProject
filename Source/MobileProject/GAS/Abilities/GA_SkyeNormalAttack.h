// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SkyeNormalAttack.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_SkyeNormalAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SkyeNormalAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | Montage")
	class UAnimMontage* FirstHitMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | GE")
	TSubclassOf<class UGameplayEffect> GE_FirstHit;

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	void ApplyFirstHitEffect();
};