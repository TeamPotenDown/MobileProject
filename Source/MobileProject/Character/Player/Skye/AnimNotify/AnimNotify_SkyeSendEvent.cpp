// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SkyeSendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_SkyeSendEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !Animation) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData Data;
	Data.EventTag = FGameplayTag::RequestGameplayTag("Event.SkyeNormal.Hit");
	Data.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, FGameplayTag::RequestGameplayTag("Event.SkyeNormal.Hit"), Data);
}
