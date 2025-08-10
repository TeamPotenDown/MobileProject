#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GATA_DK_PrimaryComboOne.generated.h"

UCLASS()
class MOBILEPROJECT_API AGATA_DK_PrimaryComboOne : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGATA_DK_PrimaryComboOne();
	
	// 지름(단위 cm).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	float RangeDiameter = 200.f;

	// 어떤 오브젝트 타입을 대상으로 할지 (Pawn 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	// 선택: 특정 클래스만 타깃 허용 (nullptr면 무시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TSubclassOf<AActor> RequiredClass = nullptr;

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;

private:
	bool FindNearestActorInRadius(AActor*& OutNearest, FVector& OutOrigin, FVector& OutForward) const;
};
