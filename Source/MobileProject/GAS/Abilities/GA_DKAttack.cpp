// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_DKAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "MobileProject/Utils/LogUtils.h"

DEFINE_LOG_CATEGORY(LogMP);

UGA_DKAttack::UGA_DKAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	EventTag_ComboContinue = MP_TAG_COMBO_CONTINUE;
	EventTag_WindowOpen = MP_TAG_WINDOW_OPEN;
	EventTag_WindowClose = MP_TAG_WINDOW_CLOSE;

	AbilityTags.AddTag(MP_TAG_DK_PRIMARY_ATTACK);
}

void UGA_DKAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* AvatorCharacter = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	AvatorCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	CurrentCombo = 1;
	bCanAcceptInput = false;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ComboMontage, 1.0f, FName(TEXT("Combo1")));
	MontageTask->OnCompleted.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_DKAttack::HandleMontageCompleted);
	MontageTask->ReadyForActivation();

	WaitComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, EventTag_ComboContinue, nullptr, false, true);
	WaitComboTask->EventReceived.AddDynamic(this, &UGA_DKAttack::HandleComboEventReceived);
	WaitComboTask->ReadyForActivation();

	StartWaitWindowOpen();
}

void UGA_DKAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	if (WaitComboTask)
	{
		WaitComboTask->EndTask();
		WaitComboTask = nullptr;
	}
	if (WaitWindowTask)
	{
		WaitWindowTask->EndTask();
		WaitWindowTask = nullptr;
	}

	ACharacter* AvatorCharacter = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	AvatorCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_DKAttack::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_DKAttack::HandleComboEventReceived(FGameplayEventData Payload)
{
	if (!bCanAcceptInput)
	{
		return;
	}

	if (CurrentCombo >= MaxComboCount)
	{
		return;
	}

	// 연속으로 들어왔을 때 CurrentCombo를 증가시키지 못하도록 방어
	bCanAcceptInput = false;
	StartWaitWindowOpen();

	++CurrentCombo;

	const FName MontageSectionName = *FString::Printf(TEXT("Combo%d"), CurrentCombo);
	MontageJumpToSection(MontageSectionName);
}

void UGA_DKAttack::HandleWindowEventReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag == EventTag_WindowOpen)
	{
		// 창이 열리면 콤보 입력을 받을 수 있도록 true로 설정
		bCanAcceptInput = true;
		// 창이 닫히기를 기다린다.
		StartWaitWindowClose();
	}
	else if (Payload.EventTag == EventTag_WindowClose)
	{
		// 창이 닫히면 콤보 입력을 받을 수 없도록 false로 설정
		bCanAcceptInput = false;
		// 창이 열리기를 기다린다.
		StartWaitWindowOpen();
	}
}

void UGA_DKAttack::StartWaitWindowOpen()
{
	if (WaitWindowTask)
	{
		WaitWindowTask->EndTask();
	}
	WaitWindowTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, EventTag_WindowOpen, nullptr, false, true);
	WaitWindowTask->EventReceived.AddDynamic(this, &UGA_DKAttack::HandleWindowEventReceived);
	WaitWindowTask->ReadyForActivation();
}

void UGA_DKAttack::StartWaitWindowClose()
{
	if (WaitWindowTask)
	{
		WaitWindowTask->EndTask();
	}
	WaitWindowTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, EventTag_WindowClose, nullptr, false, true);
	WaitWindowTask->EventReceived.AddDynamic(this, &UGA_DKAttack::HandleWindowEventReceived);
	WaitWindowTask->ReadyForActivation();
}
