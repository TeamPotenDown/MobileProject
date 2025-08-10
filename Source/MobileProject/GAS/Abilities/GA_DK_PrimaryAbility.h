// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_DK_GameplayAbilityBase.h"
#include "Abilities/GameplayAbility.h"
#include "GA_DK_PrimaryAbility.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UGA_DK_PrimaryAbility : public UGA_DK_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_DK_PrimaryAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 입력 감지 콜백 */
	UFUNCTION()
	void OnPressed();
	/** 콤보 공격 실행시 콜백, GA 실행 */
	UFUNCTION()
	void OnCobmoTriggered(FGameplayEventData Payload);
	
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
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask = nullptr; // 몽타주 재생을 위한 AT
	UPROPERTY()
	TObjectPtr<class UAbilityTask_ContinuousDetectInputPress> InputPressTask = nullptr; // 입력을 지속적으로 감시하는 AT
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitAttackTriggerEvent = nullptr; // 공격 이벤트를 감시하는 AT
	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask = nullptr; // 타겟 데이터를 기다리는 AT

	/* GE */
	UPROPERTY()
	TSubclassOf<class UGameplayEffect> WindowEffectClass = nullptr;

	/** 콜백  */
	UFUNCTION()
	void HandleMontageCompleted();

	/** TA로 미리 확보한 데이터 저장 */
	FGameplayAbilityTargetDataHandle SavedTargetData;

protected:
	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void ActivateTargetActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|ComboEvent", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, TSubclassOf<class AGameplayAbilityTargetActor>> TargetActorClasses; // 콤보 공격용 타겟 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|ComboEvent", meta = (AllowPrivateAccess = "true"))
	TMap<int32, FGameplayTag> ComboEventTags; // 콤보 이벤트 태그 매핑 (콤보 인덱스 -> 태그)
};
