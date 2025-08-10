// RangerMeleeAttack.cpp
#include "RangerMeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "MobileProject/Character/Player/Ranger/PlayerRanger.h"
#include "MobileProject/GAS/Components/CRAttributeSet.h"
#include "Particles/ParticleSystemComponent.h"

URangerMeleeAttack::URangerMeleeAttack()
{
    // 어빌리티 기본 설정
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    // 태그 설정
    //AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.BasicAttack")));
    //ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
    
    // 블록 태그
    //ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
    //ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
}

void URangerMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 애니메이션 재생
    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (Character && AttackMontage)
    {
        Character->PlayAnimMontage(AttackMontage);
    }

    // 투사체 발사
    FireProjectile();

    // 어빌리티 종료
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void URangerMeleeAttack::FireProjectile()
{
    if (!ProjectileClass)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        return;
    }

    // 발사 위치 계산
    FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorRotation().RotateVector(MuzzleOffset);
    
    // 발사 방향 계산
    FVector FireDirection = GetFireDirection();
    FRotator SpawnRotation = FireDirection.Rotation();

    // 스폰 파라미터
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Character;
    SpawnParams.Instigator = Character;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 투사체 스폰
    ACRProjectileBase* Projectile = GetWorld()->SpawnActor<ACRProjectileBase>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Projectile)
    {
        // 투사체 데이터 설정
        Projectile->SetProjectileData(ProjectileData);

        // 데미지 효과 설정
        if (DamageGameplayEffectClass)
        {
            // 데미지 계산
            const UCRAttributeSet* AttributeSet = Cast<UCRAttributeSet>(GetAbilitySystemComponentFromActorInfo()->GetAttributeSet(UCRAttributeSet::StaticClass()));
            float BaseDamage = AttributeSet ? AttributeSet->GetAttackDamage() : 20.0f;
            float FinalDamage = BaseDamage * DamageMultiplier;

            // 크리티컬 확률 체크
            float CritChance = AttributeSet ? AttributeSet->GetCriticalChance() : 0.0f;
            bool bIsCritical = FMath::RandRange(0.0f, 100.0f) <= CritChance;
            
            if (bIsCritical)
            {
                float CritMultiplier = AttributeSet ? AttributeSet->GetCriticalDamage() : 2.0f;
                FinalDamage *= CritMultiplier;
            }

            // GameplayEffect Spec 생성
            FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
            EffectContext.AddSourceObject(Character);
            EffectContext.AddHitResult(FHitResult(), true); // 크리티컬 정보
            
            FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
                DamageGameplayEffectClass,
                GetAbilityLevel(),
                EffectContext
            );

            if (SpecHandle.IsValid())
            {
                // 데미지 값 설정
                // SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                //     FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
                //     FinalDamage
                // );
                
                Projectile->SetEffectSpecHandle(SpecHandle);
            }
        }

        // 투사체 발사
        Projectile->FireProjectile(FireDirection, Character, GetAbilitySystemComponentFromActorInfo());
    }

    // 발사 효과
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, SpawnLocation);
    }

    if (MuzzleFlashParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            MuzzleFlashParticle,
            SpawnLocation,
            SpawnRotation
        );
    }
}

FVector URangerMeleeAttack::GetFireDirection() const
{
    APlayerRanger* PlayerCharacter = Cast<APlayerRanger>(GetAvatarActorFromActorInfo());
    if (PlayerCharacter)
    {
        // 플레이어 캐릭터의 공격 방향 사용
        return PlayerCharacter->GetAttackDirection();
    }

    // 기본적으로 캐릭터가 바라보는 방향
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Character)
    {
        return Character->GetActorForwardVector();
    }

    return FVector::ForwardVector;
}

AActor* URangerMeleeAttack::FindTarget() const
{
    APlayerRanger* PlayerCharacter = Cast<APlayerRanger>(GetAvatarActorFromActorInfo());
    if (PlayerCharacter)
    {
        return PlayerCharacter->GetCurrentTarget();
    }

    // 가장 가까운 타겟 찾기
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        return nullptr;
    }

    float MinDistance = AutoTargetRange;
    AActor* NearestTarget = nullptr;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Character->GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(AutoTargetRange),
        QueryParams
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* PotentialTarget = Overlap.GetActor();
        
        if (PotentialTarget && PotentialTarget->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), PotentialTarget->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestTarget = PotentialTarget;
            }
        }
    }

    return NearestTarget;
}