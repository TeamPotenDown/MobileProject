// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SkyeDash.generated.h"

/**
 * 3초 이내 재사용 시 시전 위치로 복귀 후 쿨타임 시작 (3초)
 * 3초 이내 재사용하지 않을 시 쿨타임 시작 (3초)
 */
UCLASS()
class MOBILEPROJECT_API UGA_SkyeDash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SkyeDash();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDistance  = 900.f; // 대시거리
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDuration  = 0.18f; // 대시 이동 소요시간
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float ReturnDuration= 0.16f; // 3초이내 복귀 소요시간

	UPROPERTY(EditDefaultsOnly, Category="Dash|Effects")
	TSubclassOf<UGameplayEffect> ReturnWindowEffects;
	UPROPERTY(EditDefaultsOnly, Category="Dash|Effects")
	TSubclassOf<UGameplayEffect> CooldownEffects;
	
protected:
	// 서버 기준 시작 위치(복귀 위치)
	UPROPERTY()
	FVector OriginAuthority = FVector::ZeroVector;
	// 클라 예측용 시작 위치
	UPROPERTY()
	FVector OriginPredicted = FVector::ZeroVector;
	
	// 반환 창 GE 핸들(서버 전용)
	FActiveGameplayEffectHandle ReturnWindowHandle;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_ApplyRootMotionMoveToForce> DashTask;

	UFUNCTION()
	void OnDash_MoveFinished();
	UFUNCTION()
	void OnReturn_MoveFinished();
	UFUNCTION()
	void OnReturnWindowRemoved(const FGameplayEffectRemovalInfo& Info); // 서버

	UFUNCTION()
	static FVector GetDashDirTopDown(const ACharacter* Char);
	UFUNCTION()
	void ApplyCooldown();
};
