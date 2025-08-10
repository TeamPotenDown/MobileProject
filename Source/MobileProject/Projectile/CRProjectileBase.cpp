// CRProjectileBase.cpp
#include "CRProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

ACRProjectileBase::ACRProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌 컴포넌트
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(20.0f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1); // Projectile 채널
    CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
    RootComponent = CollisionComp;

    // 이동 컴포넌트
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 2000.0f;
    ProjectileMovement->MaxSpeed = 2000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    // 파티클 컴포넌트
    ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
    ParticleComp->SetupAttachment(RootComponent);

    // 네트워크 복제
    bReplicates = true;
    SetReplicateMovement(true);
}

void ACRProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    // 충돌 이벤트 바인딩
    CollisionComp->OnComponentHit.AddDynamic(this, &ACRProjectileBase::OnProjectileHit);

    // 시작 위치 저장
    StartLocation = GetActorLocation();

    // 생존 시간 타이머
    GetWorldTimerManager().SetTimer(LifeTimeTimerHandle, this, &ACRProjectileBase::DestroyProjectile, ProjectileData.LifeTime, false);

    // Homing 타입일 경우 타겟 업데이트 시작
    if (ProjectileData.ProjectileType == EProjectileType::Homing)
    {
        GetWorldTimerManager().SetTimer(HomingUpdateTimerHandle, this, &ACRProjectileBase::UpdateHomingTarget, 0.1f, true);
    }
}

void ACRProjectileBase::FireProjectile(const FVector& Direction, APawn* InInstigator, UAbilitySystemComponent* InInstigatorASC)
{
    SetInstigator(InInstigator);
    InstigatorASC = InInstigatorASC;

    // 속도 설정
    ProjectileMovement->InitialSpeed = ProjectileData.Speed;
    ProjectileMovement->MaxSpeed = ProjectileData.Speed;
    ProjectileMovement->Velocity = Direction * ProjectileData.Speed;

    // Homing 설정
    if (ProjectileData.ProjectileType == EProjectileType::Homing)
    {
        ProjectileMovement->bIsHomingProjectile = true;
        ProjectileMovement->HomingAccelerationMagnitude = ProjectileData.HomingAcceleration;
    }
}

void ACRProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    // 자기 자신이나 발사한 액터는 무시
    if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
    {
        return;
    }

    // 이미 맞은 액터인지 확인 (관통 시)
    if (HitActors.Contains(OtherActor))
    {
        return;
    }

    // 거리 체크
    float TravelDistance = FVector::Dist(StartLocation, GetActorLocation());
    if (TravelDistance > ProjectileData.MaxRange)
    {
        DestroyProjectile();
        return;
    }

    // 타입별 처리
    switch (ProjectileData.ProjectileType)
    {
    case EProjectileType::Normal:
        HandleNormalHit(OtherActor, Hit);
        break;
    case EProjectileType::Penetrating:
        HandlePenetratingHit(OtherActor, Hit);
        break;
    case EProjectileType::Explosive:
        HandleExplosiveHit(Hit);
        break;
    case EProjectileType::Homing:
        HandleNormalHit(OtherActor, Hit);
        break;
    }
}

void ACRProjectileBase::HandleNormalHit(AActor* HitActor, const FHitResult& Hit)
{
    // 데미지 적용
    ApplyDamageToTarget(HitActor);

    // 투사체 파괴
    DestroyProjectile();
}

void ACRProjectileBase::HandlePenetratingHit(AActor* HitActor, const FHitResult& Hit)
{
    // 맞은 액터 기록
    HitActors.Add(HitActor);

    // 데미지 적용
    ApplyDamageToTarget(HitActor);

    // 관통 횟수 증가
    CurrentPenetrationCount++;

    // 최대 관통 횟수 도달 시 파괴
    if (CurrentPenetrationCount >= ProjectileData.PenetrationCount)
    {
        DestroyProjectile();
    }
}

void ACRProjectileBase::HandleExplosiveHit(const FHitResult& Hit)
{
    // 폭발 데미지 적용
    ApplyExplosiveDamage(Hit.Location);

    // 투사체 파괴
    DestroyProjectile();
}

void ACRProjectileBase::ApplyDamageToTarget(AActor* Target)
{
    if (!Target || !InstigatorASC)
    {
        return;
    }

    //UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
    if (!TargetASC)
    {
        return;
    }

    // GameplayEffect 적용
    if (DamageEffectSpecHandle.IsValid())
    {
        InstigatorASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);
    }
    else if (DamageEffectClass)
    {
        FGameplayEffectContextHandle EffectContext = InstigatorASC->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(DamageEffectClass, 1, EffectContext);
        if (SpecHandle.IsValid())
        {
            InstigatorASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }
    }
}

void ACRProjectileBase::ApplyExplosiveDamage(const FVector& ExplosionLocation)
{
    if (!InstigatorASC)
    {
        return;
    }

    // 폭발 범위 내 액터 찾기
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetInstigator());

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        ExplosionLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(ProjectileData.ExplosionRadius),
        QueryParams
    );

    // 각 액터에 데미지 적용
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (HitActor)
        {
            ApplyDamageToTarget(HitActor);
        }
    }

    // 디버그 표시
#if ENABLE_DRAW_DEBUG
    DrawDebugSphere(GetWorld(), ExplosionLocation, ProjectileData.ExplosionRadius, 32, FColor::Red, false, 2.0f);
#endif
}

void ACRProjectileBase::UpdateHomingTarget()
{
    if (!HomingTarget || !IsValid(HomingTarget))
    {
        // 가장 가까운 적 찾기
        float MinDistance = 10000.0f;
        AActor* NewTarget = nullptr;

        TArray<FOverlapResult> Overlaps;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(GetInstigator());

        GetWorld()->OverlapMultiByChannel(
            Overlaps,
            GetActorLocation(),
            FQuat::Identity,
            ECollisionChannel::ECC_Pawn,
            FCollisionShape::MakeSphere(2000.0f), // 탐색 반경
            QueryParams
        );

        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* PotentialTarget = Overlap.GetActor();
            if (PotentialTarget && PotentialTarget->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
            {
                float Distance = FVector::Dist(GetActorLocation(), PotentialTarget->GetActorLocation());
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    NewTarget = PotentialTarget;
                }
            }
        }

        HomingTarget = NewTarget;
        if (HomingTarget && ProjectileMovement)
        {
            ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
        }
    }
}

void ACRProjectileBase::DestroyProjectile()
{
    // 타이머 정리
    GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
    GetWorldTimerManager().ClearTimer(HomingUpdateTimerHandle);

    // 파괴
    Destroy();
}