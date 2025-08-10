#include "AbilityTask_ContinuousDetectInputPress.h"

#include "AbilitySystemComponent.h"

void UAbilityTask_ContinuousDetectInputPress::OnPressCallback()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	// We are done. Kill us so we don't keep getting broadcast messages
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnInputPressedContinuous.Broadcast();
	}
}

void UAbilityTask_ContinuousDetectInputPress::Activate()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC || !Ability)
	{
		return ;
	}

	// 바로 실행하는 거면 바로 실행
	if (bTestInitialState && IsLocallyControlled())
	{
		FGameplayAbilitySpec* Spec = Ability->GetCurrentAbilitySpec();
		if (Spec && Spec->InputPressed)
		{
			OnPressCallback();
			return ;
		}
	}

	// 입력 감시 시작
	DelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UAbilityTask_ContinuousDetectInputPress::OnPressCallback);
	if (IsForRemoteClient())
	{
		if (!ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

UAbilityTask_ContinuousDetectInputPress* UAbilityTask_ContinuousDetectInputPress::WaitInputPress(UGameplayAbility* OwningAbility,
	bool bInTestInitialState)
{
	UAbilityTask_ContinuousDetectInputPress* Task = NewAbilityTask<UAbilityTask_ContinuousDetectInputPress>(OwningAbility);
	Task->bTestInitialState = bInTestInitialState;
	
	return Task;
}
