#pragma once

#include "CoreMinimal.h"
#include "GA_DK_GameplayAbilityBase.h"
#include "GA_DK_PassiveTargetingAbility.generated.h"

UCLASS()
class MOBILEPROJECT_API UGA_DK_PassiveTargetingAbility : public UGA_DK_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_DK_PassiveTargetingAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);
	/** 콤보 공격 전에 TA 활성화 */
	UFUNCTION()
	void ActivateTargetActor();
	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (AllowPrivateAccess = true))
	TSubclassOf<class AGameplayAbilityTargetActor> TargetActorClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (AllowPrivateAccess = true))
	float TargetingRange = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UAbilityTask_WaitTargetData> ContinuousWaitTargetDataTask = nullptr;
};
