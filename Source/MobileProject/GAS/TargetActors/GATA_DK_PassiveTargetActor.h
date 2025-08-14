#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GATA_DK_PassiveTargetActor.generated.h"

UCLASS()
class MOBILEPROJECT_API AGATA_DK_PassiveTargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGATA_DK_PassiveTargetActor();

public:
	virtual void Tick(float DeltaTime) override;

	virtual void StartTargeting(UGameplayAbility* Ability);
	virtual void ConfirmTargetingAndContinue();

protected:
	void BroadcastIfChanged();
	bool GatherBySphere(class AActor* Source, TArray<AActor*>& OutOverlappedActors) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	float RangeDiameter = 200.f; // 지름 (단위 cm)

	/** 어떤 오브젝트 타입을 대상으로 할지 (Pawn 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** 선택: 특정 클래스만 타깃 허용 (nullptr면 무시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TSubclassOf<class AActor> RequiredClass = nullptr;

private:
	bool GetMoveDir(class AActor* Source, FVector& OutDirection);
	class AActor* PickDirectionalNearest(class AActor* Source, const TArray<class AActor*>& OverlappedActors, const FVector& MoveDir);
	class AActor* PickNearest(class AActor* Source, const TArray<class AActor*>& OverlappedActors);
	
	UPROPERTY()
	TWeakObjectPtr<class AActor> LastTarget;
	UPROPERTY()
	TWeakObjectPtr<class AActor> NewTarget;
	/** 최소 내적 값 */
	UPROPERTY()
	float ForwardDotMin = 0.2f;
	/** 각도에 따른 패널티 비율 */
	UPROPERTY()
	float PenaltyRate = 50.0f;
};
