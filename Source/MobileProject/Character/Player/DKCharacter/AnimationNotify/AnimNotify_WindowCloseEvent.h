// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "AnimNotify_WindowCloseEvent.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEPROJECT_API UAnimNotify_WindowCloseEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("WindowCloseEvent");
	}
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|GameplayEffect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGameplayEffect> GameplayEffectClass = nullptr;
};
