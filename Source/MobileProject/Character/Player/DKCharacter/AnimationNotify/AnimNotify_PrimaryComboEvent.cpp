#include "AnimNotify_PrimaryComboEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_PrimaryComboEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner->GetWorld()->IsGameWorld())
	{
		return ;
	}

	FGameplayEventData Payload;
	Payload.EventTag = TriggerTag;
	Payload.Instigator = Owner;
	Payload.Target = Owner;
	Payload.TargetTags.AddTag(ComboEventTag);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Payload.EventTag, Payload);
}
