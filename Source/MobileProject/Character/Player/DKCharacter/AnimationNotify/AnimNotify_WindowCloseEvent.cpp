// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_WindowCloseEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_WindowCloseEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (ASC)
	{
		FGameplayEffectSpecHandle GESpecHandle = ASC->MakeOutgoingSpec(GameplayEffectClass, 1, ASC->MakeEffectContext());
		ASC->ApplyGameplayEffectSpecToSelf(*GESpecHandle.Data.Get());
	}
}
