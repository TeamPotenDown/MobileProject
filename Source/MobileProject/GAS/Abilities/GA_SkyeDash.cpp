// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SkyeDash.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/RepLayout.h"

UGA_SkyeDash::UGA_SkyeDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = false;
	
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("InputTag.Dash"));
}

void UGA_SkyeDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo, nullptr))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ACharacter* Char = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	
	const bool bHasReturnWindow = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dash.ReturnWindow")));

	// 3초 반환 창이 열려있다면, 원위치 복귀
	if (bHasReturnWindow)
	{
		const FVector TargetDir = ActorInfo->IsNetAuthority() ? OriginAuthority : OriginPredicted;

		DashTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
			this, FName(TEXT("DashReturn")), TargetDir, ReturnDuration, false, MOVE_Walking, false,
			nullptr, ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0.0f);
		DashTask->OnTimedOut.AddDynamic(this, &UGA_SkyeDash::OnReturn_MoveFinished);
		DashTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UGA_SkyeDash::OnReturn_MoveFinished);
		DashTask->ReadyForActivation();

		return;
	}
	else
	{
	    // 3초 반환 창이 열려있지 않을때 (첫 입력), 대시 시작하고 창을 열자
	    const FVector OriginDir = Char->GetActorLocation();
		OriginPredicted = OriginDir; // 클라
		if (ActorInfo->IsNetAuthority())
		{
			OriginAuthority = OriginDir; // 서버
		}

		const FVector Dir = GetDashDirTopDown(Char);
		const FVector TargetDir = OriginDir + (Dir * DashDistance);
		
		DashTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
					this, FName(TEXT("DashForward")), TargetDir, ReturnDuration, false, MOVE_Walking, false,
					nullptr, ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0.0f);
		DashTask->OnTimedOut.AddDynamic(this, &UGA_SkyeDash::OnDash_MoveFinished);
		DashTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UGA_SkyeDash::OnDash_MoveFinished);
		DashTask->ReadyForActivation();

		// 서버: 3초 반환 창 GE 적용(+ 만료/제거 콜백에서 쿨타임 시작)
		if (ActorInfo->IsNetAuthority())
		{
			checkf(ReturnWindowEffects != nullptr, TEXT("ReturnWindow GE missing"));

			FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(ReturnWindowEffects, 1.f);
			if (!Spec.IsValid()) return;
			
			ReturnWindowHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec.Data.Get());
			if (!ReturnWindowHandle.IsValid()) return;

			if (FOnActiveGameplayEffectRemoved_Info* Removed =
			ActorInfo->AbilitySystemComponent->OnGameplayEffectRemoved_InfoDelegate(ReturnWindowHandle))
			{
				// 포인터이므로 -> 로 AddUObject 호출
				FDelegateHandle ReturnWindowRemovedDelegateHandle =
					Removed->AddUObject(this, &UGA_SkyeDash::OnReturnWindowRemoved);
			}
			else
			{
				ensureMsgf(false, TEXT("OnGameplayEffectRemoved_InfoDelegate() returned null for handle"));
			}
			return;
		}
	}
}

void UGA_SkyeDash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkyeDash::OnDash_MoveFinished()
{
	// 첫 대시 종료 시 GA는 바로 종료한다
	// 반환 창은 GE가 3초간 유지하며, 재시전 분기는 GE 태그로 판정
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkyeDash::OnReturn_MoveFinished()
{
	// 복귀 완료시 서버에서 반환 창 제거 후 쿨타임을 시작한다.
	if (CurrentActorInfo && CurrentActorInfo->IsNetAuthority())
	{
		if (!ReturnWindowHandle.IsValid()) return;
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		if (!ASC) return;

		ASC->RemoveActiveGameplayEffect(ReturnWindowHandle);
		ReturnWindowHandle = FActiveGameplayEffectHandle();
		ApplyCooldown();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_SkyeDash::OnReturnWindowRemoved(const FGameplayEffectRemovalInfo& Info)
{
	// 3초 내 재시전 실패
	if (CurrentActorInfo && CurrentActorInfo->IsNetAuthority())
	{
		ApplyCooldown();
		ReturnWindowHandle = FActiveGameplayEffectHandle();
	}
}

FVector UGA_SkyeDash::GetDashDirTopDown(const ACharacter* Char)
{
	if (!Char) return FVector::ForwardVector;

	const UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();

	// 속도 있으면 그 방향으로
	if (MoveComp)
	{
		const FVector Vel = FVector(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0);
		if (!Vel.IsNearlyZero())
		{
			FVector Dir = Vel.GetSafeNormal();

			const FVector Floor = MoveComp->CurrentFloor.HitResult.ImpactNormal;
			if (!Floor.IsNearlyZero())
			{
				Dir = FVector::VectorPlaneProject(Dir, Floor).GetSafeNormal();
			}
			return Dir;
		}
	}

	// 정지 중이면 캐릭터 정면으로
	const FVector Planar = Char->GetActorForwardVector();
	const FVector Forward = FVector(Planar.X, Planar.Y, 0).GetSafeNormal();
	return Forward.IsNearlyZero() ? FVector::ForwardVector : Forward;
}

void UGA_SkyeDash::ApplyCooldown()
{
	checkf(CooldownEffects != nullptr, TEXT("CooldownEffects must be valid"));

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(CooldownEffects, 1.f);
	if (!Spec.IsValid()) return;
	ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, Spec.Data.Get());
}
