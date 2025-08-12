#include "GA_SkyeComboAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"

UGA_SkyeComboAttack::UGA_SkyeComboAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 로컬 예측 + 서버 확정: 기본 근접 공격에 무난
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 예측/롤백 흐름과 결합이 약하고, 불필요한 RPC가 늘 수 있기 때문에 false로 설정할 것을 권장
	// TODO: 변경
	bReplicateInputDirectly = true;
	
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("InputTag.PrimaryAttack.Normal"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.State.Attack.Combo"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Character.State.Attack.Charging"));
}

void UGA_SkyeComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	CurrentComboIndex = 0;
	bInputWindowOpen = false;
	bInputNext = false;

	// 첫 섹션 시작
	StartFirstOrNextSection();
}

void UGA_SkyeComboAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	ClearSectionEvents();
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_SkyeComboAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearSectionEvents();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkyeComboAttack::InputPressed(const FGameplayAbilitySpecHandle Handle,
 	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
 {
 	if (bInputWindowOpen)
 	{
 		bInputNext = true;
 	}
 }

void UGA_SkyeComboAttack::StartFirstOrNextSection()
{
	bSectionEnding = false;
	
	++CurrentComboIndex;
	FName SectionName = FName(*FString::Printf(TEXT("Combo%d"), CurrentComboIndex));

	// 섹션 바인딩 준비
	BindSectionEvents();
	
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ComboActionMontage, 1.f, SectionName, true);
	
	MontageTask->OnCompleted.AddDynamic(this, &UGA_SkyeComboAttack::OnSectionEnded); // 정상종료
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_SkyeComboAttack::OnSectionEnded); // 끝나기 직전
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_SkyeComboAttack::OnSectionEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_SkyeComboAttack::OnSectionEnded);

	MontageTask->ReadyForActivation();
}

void UGA_SkyeComboAttack::BindSectionEvents()
{
	// 섹션마다 새로 바인딩 (기존 바인딩 제거)
	ClearSectionEvents();
	
	// 히트 타이밍 이벤트
	HitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.SkyeNormal.Hit"), nullptr, false, false);
	HitEventTask->EventReceived.AddDynamic(this, &UGA_SkyeComboAttack::OnHitEventReceived);
	HitEventTask->ReadyForActivation();
	
	// 입력창 열림/닫힘 이벤트
	InputOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.SkyeNormal.InputOpen"), nullptr, false, false);
	InputOpenTask->EventReceived.AddDynamic(this, &UGA_SkyeComboAttack::OnInputOpenReceived);
	InputOpenTask->ReadyForActivation();
	
	InputCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Event.SkyeNormal.InputClose"), nullptr, false, false);
	InputCloseTask->EventReceived.AddDynamic(this, &UGA_SkyeComboAttack::OnInputCloseReceived);
	InputCloseTask->ReadyForActivation();
}

void UGA_SkyeComboAttack::ClearSectionEvents()
{
	if (HitEventTask) { HitEventTask->EndTask(); HitEventTask = nullptr; }
	if (InputOpenTask) { InputOpenTask->EndTask(); InputOpenTask = nullptr; }
	if (InputCloseTask) { InputCloseTask->EndTask(); InputCloseTask = nullptr; }
}

void UGA_SkyeComboAttack::OnSectionEnded()
{
	if (bSectionEnding) return; // 섹션 종료 가드
	bSectionEnding = true;
	
	bInputWindowOpen = false;

	const bool bHasNextCombo = (CurrentComboIndex < MaxComboCount);
	if (bHasNextCombo && bInputNext)
	{
		bInputNext = false;
		bSectionEnding = false;
		StartFirstOrNextSection();
		return;
	}

	// 더 진행하지 않으면 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SkyeComboAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	if (Payload.OptionalObject != ComboActionMontage) return;
	// 서버에서 데미지 확정
	if (CurrentActorInfo && HasAuthority(&CurrentActivationInfo))
	{
		ApplyComboDamage();
	}
}

void UGA_SkyeComboAttack::OnInputOpenReceived(FGameplayEventData Payload)
{
	if (Payload.OptionalObject != ComboActionMontage) return;
	bInputWindowOpen = true;
}

void UGA_SkyeComboAttack::OnInputCloseReceived(FGameplayEventData Payload)
{
	if (Payload.OptionalObject != ComboActionMontage) return;
	bInputWindowOpen = false;
}

void UGA_SkyeComboAttack::ApplyComboDamage()
{
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	ACharacter* Owner = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Owner) return;

	FVector StartLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector()*50.f;
	FVector EndLocation = StartLocation + Owner->GetActorForwardVector()*100.f;
	
	TArray<FHitResult> HitResults;
	FCollisionShape Box = FCollisionShape::MakeBox(FVector(50.f));
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, StartLocation, EndLocation, FQuat::Identity,
		ECC_Visibility, Box, Params);
	
	DrawDebugBox(GetWorld(), EndLocation, Box.GetExtent(), FQuat::Identity, FColor::Red, false, 1.0f);

	// 이번 타의 GE (인덱스: 0,1,2)
	const int32 GEIndex = FMath::Clamp(CurrentComboIndex - 1, 0, ComboHitEffects.Num() - 1);
	TSubclassOf<UGameplayEffect> Effect = ComboHitEffects[GEIndex];
	if (!Effect) return;
	
	for (const FHitResult& Hit : HitResults)
	{
		ApplyEffectToTarget(Hit.GetActor(), Effect);
	}
}

void UGA_SkyeComboAttack::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> EffectClass) const
{
	if (!TargetActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass, 1.f);
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
