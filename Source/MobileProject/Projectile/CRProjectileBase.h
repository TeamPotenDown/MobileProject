// CRProjectileBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "CRProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Penetrating UMETA(DisplayName = "Penetrating"),
    Explosive   UMETA(DisplayName = "Explosive"),
    Homing      UMETA(DisplayName = "Homing")
};

USTRUCT(BlueprintType)
struct FProjectileData
{
    GENERATED_BODY()

    // 투사체 속도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float Speed = 2000.0f;

    // 최대 비행 거리
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float MaxRange = 2000.0f;

    // 최대 생존 시간
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float LifeTime = 3.0f;

    // 투사체 타입
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    EProjectileType ProjectileType = EProjectileType::Normal;

    // 관통 가능 횟수 (Penetrating 타입일 때)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "ProjectileType == EProjectileType::Penetrating"))
    int32 PenetrationCount = 3;

    // 폭발 반경 (Explosive 타입일 때)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "ProjectileType == EProjectileType::Explosive"))
    float ExplosionRadius = 300.0f;

    // 유도 강도 (Homing 타입일 때)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "ProjectileType == EProjectileType::Homing"))
    float HomingAcceleration = 5000.0f;

    FProjectileData()
    {
    }
};

UCLASS()
class MOBILEPROJECT_API ACRProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    ACRProjectileBase();

protected:
    virtual void BeginPlay() override;

public:
    // 투사체 발사
    virtual void FireProjectile(const FVector& Direction, APawn* InInstigator, UAbilitySystemComponent* InInstigatorASC);

    // 투사체 데이터 설정
    void SetProjectileData(const FProjectileData& InData) { ProjectileData = InData; }

    // GameplayEffect 설정
    void SetDamageEffectClass(TSubclassOf<UGameplayEffect> InDamageEffect) { DamageEffectClass = InDamageEffect; }
    void SetEffectSpecHandle(const FGameplayEffectSpecHandle& InSpecHandle) { DamageEffectSpecHandle = InSpecHandle; }

protected:
    // 충돌 처리
    UFUNCTION()
    virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    // 타입별 충돌 처리
    virtual void HandleNormalHit(AActor* HitActor, const FHitResult& Hit);
    virtual void HandlePenetratingHit(AActor* HitActor, const FHitResult& Hit);
    virtual void HandleExplosiveHit(const FHitResult& Hit);

    // 데미지 적용
    virtual void ApplyDamageToTarget(AActor* Target);

    // 폭발 데미지 (Explosive 타입)
    virtual void ApplyExplosiveDamage(const FVector& ExplosionLocation);

    // 투사체 파괴
    virtual void DestroyProjectile();

    // 유도 타겟 업데이트 (Homing 타입)
    void UpdateHomingTarget();

protected:
    // === Components ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* ParticleComp;

    // === 투사체 데이터 ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    FProjectileData ProjectileData;

    // === GameplayEffect ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY()
    FGameplayEffectSpecHandle DamageEffectSpecHandle;

    // === 런타임 데이터 ===
    UPROPERTY()
    UAbilitySystemComponent* InstigatorASC;

    // 시작 위치 (거리 계산용)
    FVector StartLocation;

    // 현재 관통 횟수
    int32 CurrentPenetrationCount;

    // 이미 맞은 액터들 (관통 시 중복 방지)
    UPROPERTY()
    TArray<AActor*> HitActors;

    // 유도 타겟 (Homing 타입)
    UPROPERTY()
    AActor* HomingTarget;

    // 타이머 핸들
    FTimerHandle LifeTimeTimerHandle;
    FTimerHandle HomingUpdateTimerHandle;
};