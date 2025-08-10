#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MobileProject/Projectile/CRProjectileBase.h"
#include "RangerMeleeAttack.generated.h"

class ACRProjectileBase;

UCLASS()
class MOBILEPROJECT_API URangerMeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	URangerMeleeAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 투사체 발사
	void FireProjectile();

	// 공격 방향 계산
	FVector GetFireDirection() const;

	// 타겟 찾기
	AActor* FindTarget() const;

protected:
	// 투사체 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ACRProjectileBase> ProjectileClass;

	// 투사체 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FProjectileData ProjectileData;

	// 데미지 효과
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	// 데미지 계수 (AttributeSet의 AttackDamage에 곱해짐)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageMultiplier = 1.0f;

	// 발사 위치 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FVector MuzzleOffset = FVector(100.0f, 0.0f, 0.0f);

	// 자동 타겟팅 범위
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float AutoTargetRange = 800.0f;

	// 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	// 발사 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* FireSound;

	// 발사 파티클
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* MuzzleFlashParticle;
};