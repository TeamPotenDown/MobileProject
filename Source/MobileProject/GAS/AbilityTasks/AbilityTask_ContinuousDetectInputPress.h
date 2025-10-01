// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_ContinuousDetectInputPress.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInputPressedContinuousSig);

/**
 * Input을 지속적/단발적으로 감시하는 AT
 *
 * 지속적으로 하고 싶으면
 * bDetectContinuous true로 설정
 * 
 * 네트워크에서도 해당 입력을 감시할 수 있게 함.
 * 즉, 서버도 해당 작업을 병렬적으로 실행하도록 함.
 *
 * 만약에, 바로 실행하고 싶으면 bTestInitialState를 true로 설정하여 바로 실행하게 끔 함.
 * 
 */
UCLASS()
class MOBILEPROJECT_API UAbilityTask_ContinuousDetectInputPress : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FInputPressedContinuousSig OnInputPressedContinuous;

	UFUNCTION()
	void OnPressCallback();

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_ContinuousDetectInputPress* WaitInputPress(UGameplayAbility* OwningAbility, bool bInTestInitialState = false);
	
protected:

	bool bTestInitialState = false;
	FDelegateHandle DelegateHandle;
};
