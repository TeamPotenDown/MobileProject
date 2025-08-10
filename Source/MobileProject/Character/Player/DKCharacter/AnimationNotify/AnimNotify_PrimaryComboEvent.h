#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "AnimNotify_PrimaryComboEvent.generated.h"

UCLASS()
class MOBILEPROJECT_API UAnimNotify_PrimaryComboEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("PrimaryComboEvent");
	}

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|ComboEvent", meta = (AllowPrivateAccess = "true"))
	FGameplayTag ComboEventTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|ComboEvent", meta = (AllowPrivateAccess = "true"))
	FGameplayTag TriggerTag = MP_TAG_DK_EVENT_NOTIFY_PRIMARY_ATTACK_TRIGGERED;
	
};
