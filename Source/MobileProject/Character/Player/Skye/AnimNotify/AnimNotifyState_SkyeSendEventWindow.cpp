// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SkyeSendEventWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotifyState_SkyeSendEventWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData Data;
	Data.EventTag = FGameplayTag::RequestGameplayTag("Event.SkyeNormal.InputOpen");
	Data.OptionalObject = Animation;
	Data.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Data.EventTag, Data);
}

void UAnimNotifyState_SkyeSendEventWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData Data;
	Data.EventTag = FGameplayTag::RequestGameplayTag("Event.SkyeNormal.InputClose");
	Data.OptionalObject = Animation;
	Data.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Data.EventTag, Data);
}
