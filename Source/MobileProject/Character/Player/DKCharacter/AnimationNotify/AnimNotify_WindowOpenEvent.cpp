// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_WindowOpenEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_WindowOpenEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
	Payload.EventTag = EventTag_WindowOpen;
	Payload.Instigator = Owner;
	Payload.Target = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner, Payload.EventTag, Payload);
}
