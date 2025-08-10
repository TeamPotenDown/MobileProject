#include "HammerActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHammerActor::AHammerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	Super::SetReplicateMovement(true);

	// == Collision Component ==
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(50.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionComponent);

	// == Mesh Component ==
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// == Projectile Movement ==
	ProjectileMove = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMove->SetUpdatedComponent(RootComponent);
	ProjectileMove->InitialSpeed = 1500.0f;
	ProjectileMove->MaxSpeed = 1500.0f;
	ProjectileMove->bRotationFollowsVelocity = true;
	ProjectileMove->ProjectileGravityScale = 0.f;
	ProjectileMove->SetAutoActivate(true);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AHammerActor::OnOverlapBegin);
}

void AHammerActor::InitProjectile(const FGameplayEffectSpecHandle& InDamageEffectSpecHandle,
	UAbilitySystemComponent* InSourceASC, FVector2D InTargetPoint, float LifeTime)
{
	DamageEffectSpecHandle = InDamageEffectSpecHandle;
	SourceASC = InSourceASC;
	TargetPoint = InTargetPoint;

	// 원하는 방향으로 날리고 싶음
	FVector Direction = FVector(TargetPoint.X, TargetPoint.Y, GetActorLocation().Z) - GetActorLocation();
	Direction.Normalize();
	// 월드 기준 보장
	ProjectileMove->bInitialVelocityInLocalSpace = false;
	ProjectileMove->Velocity = Direction * ProjectileMove->InitialSpeed;
	ProjectileMove->bRotationFollowsVelocity = true;
	
	if (SourceASC)
	{
		SetOwner(SourceASC->GetOwnerActor());
		SetInstigator(Cast<APawn>(SourceASC->GetAvatarActor()));
	}

	SetLifeSpan(LifeTime);
}

void AHammerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}
	
	// // 해당 지점까지 이동
	// const FVector2D CurrentLocation(GetActorLocation().X, GetActorLocation().Y);
	// const float Distance = FVector2D::Distance(CurrentLocation, TargetPoint);
	// if (Distance <= CollisionComponent->GetScaledSphereRadius())
	// {
	// 	ProjectileMove->StopMovementImmediately();
	// 	SetActorLocation(FVector(TargetPoint.X, TargetPoint.Y, GetActorLocation().Z));
	//
	// 	// 임팩트 Cue 실행 (GameplayCue)
	//
	// 	// Destory는 외부에서 호출되거나 LifeSpan이 끝나면 자동으로 호출함
	// }
}

void AHammerActor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AHammerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AHammerActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor || OtherActor == this || !SourceASC)
	{
		return;
	}

	// 타겟이 GAS를 가지고 있는지 확인
	if (OtherActor->Implements<UAbilitySystemInterface>())
	{
		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
			
		if (DamageEffectSpecHandle.IsValid() && TargetASC)
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
	}
}