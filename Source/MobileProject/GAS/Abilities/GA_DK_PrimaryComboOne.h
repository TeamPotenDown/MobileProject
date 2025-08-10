// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_DK_GameplayAbilityBase.h"
#include "GA_DK_PrimaryComboOne.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_DK_PrimaryComboOne : public UGA_DK_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_DK_PrimaryComboOne();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitAttackTriggerEvent = nullptr; // 공격 이벤트를 기다리는 AT
	UFUNCTION()
	void OnAttackTriggered(FGameplayEventData Payload);
	
protected:
	UFUNCTION()
	void ProjectileEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	/** 콤보 공격시 적용할 데미지 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|GameplayEffect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGameplayEffect> DamageGE = nullptr;
	/** 콤보 공격시 사용할 해머 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|GameplayEffect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AHammerActor> HammerActorClass = nullptr;

	/** 해머 액터 인스턴스 */
	UPROPERTY()
	TObjectPtr<class AHammerActor> HammerActor = nullptr;

	/** 투사체의 종료 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|GameplayEffect", meta = (AllowPrivateAccess = "true"))
	float Duration = 5.0f;

protected:
	/** 콤보 공격 전에 TA 활성화 */
	UFUNCTION()
	void ActivateTargetActor();
	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);
	
	/** TA로 미리 확보한 데이터 저장 */
	FGameplayAbilityTargetDataHandle SavedTargetData;

	/** TA를 사용하기 위한 설정 */
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask = nullptr; // 타겟 데이터를 기다리는 AT
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|TargetActor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGameplayAbilityTargetActor> TargetActorClass = nullptr; // 사용할 타겟 액터 클래스

private:
	/** 타겟 액터의 범위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|TargetActor", meta = (AllowPrivateAccess = "true"))
	float TargetActorRange = 200.0f;
};
