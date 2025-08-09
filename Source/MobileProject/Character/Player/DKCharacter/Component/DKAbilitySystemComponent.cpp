#include "DKAbilitySystemComponent.h"

#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"

UDKAbilitySystemComponent::UDKAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDKAbilitySystemComponent::AbilityLocalInputReleased(int32 InputID)
{
	if (HasMatchingGameplayTag(MP_TAG_DK_STATE_TARGETING_AWAIT_CONFIRM))
	{
		LocalInputConfirm(); 
		return;
	}
	Super::AbilityLocalInputReleased(InputID);
}

/*
 * NOTE :
 * 이 함수는 LooseTag를 사용하기 때문에 복제 안됨 (오직 클라에서 UX 목적으로 사용)
 */
void UDKAbilitySystemComponent::MarkAwaitingConfirm(bool bEnable)
{
	if (bEnable)
	{
		AddLooseGameplayTag(MP_TAG_DK_STATE_TARGETING_AWAIT_CONFIRM);
	}
	else
	{
		RemoveLooseGameplayTag(MP_TAG_DK_STATE_TARGETING_AWAIT_CONFIRM);
	}
}


