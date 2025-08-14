#pragma once

#define MP_TAG_DK_PRIMARY_ATTACK FGameplayTag::RequestGameplayTag(FName("Character.DK.PrimaryAttack"))

#define MP_TAG_DK_STATE_COMBO_WINDOW_OPEN FGameplayTag::RequestGameplayTag(FName("State.Combo.Window.Open"))
#define MP_TAG_DK_STATE_TARGETING_AWAIT_CONFIRM FGameplayTag::RequestGameplayTag(FName("State.Targeting.AwaitingConfirm"))

#define MP_TAG_DK_EVENT_NOTIFY_PRIMARY_ATTACK_TRIGGERED FGameplayTag::RequestGameplayTag(FName("Event.Notify.PrimaryAttackTrigger"))
#define MP_TAG_DK_EVENT_NOTIFY_PASSIVE_TARGETING_READY FGameplayTag::RequestGameplayTag(FName("Event.Notify.PassiveTargetingReady"))