#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "HammerActor.generated.h"

UCLASS()
class MOBILEPROJECT_API AHammerActor : public AActor
{
	GENERATED_BODY()

public:
	AHammerActor();

	void InitProjectile(
		const FGameplayEffectSpecHandle& InDamageEffectSpecHandle,
		class UAbilitySystemComponent* InSourceASC,
		// TopDown View이기 때문에 X, Y 좌표만 사용
		FVector2D InTargetPoint = FVector2D::ZeroVector,
		float LifeTime = 5.0f
		);

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Hammer", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USphereComponent> CollisionComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Hammer", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UStaticMeshComponent> MeshComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Hammer", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UProjectileMovementComponent> ProjectileMove;

	UPROPERTY(EditDefaultsOnly, Category = "Hammer", meta = (AllowPrivateAccess = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;
	UPROPERTY(EditDefaultsOnly, Category = "Hammer", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UAbilitySystemComponent> SourceASC;

	FVector2D TargetPoint = FVector2D::ZeroVector;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
