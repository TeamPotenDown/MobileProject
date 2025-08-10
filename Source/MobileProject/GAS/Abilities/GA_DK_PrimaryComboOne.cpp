#include "GA_DK_PrimaryComboOne.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MobileProject/Character/Player/DKCharacter/Hammer/HammerActor.h"
#include "MobileProject/Utils/LogUtils.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "MobileProject/GAS/TargetActors/GATA_DK_PrimaryComboOne.h"

UGA_DK_PrimaryComboOne::UGA_DK_PrimaryComboOne()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 트리거는 에디터에서 설정하자. // Event.Deulee.PrimaryAttack1 설정
}

void UGA_DK_PrimaryComboOne::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	ActivateTargetActor();
	
	WaitAttackTriggerEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this, MP_TAG_DK_EVENT_NOTIFY_PRIMARY_ATTACK_TRIGGERED, nullptr, true, true);
	if (WaitAttackTriggerEvent)
	{
		WaitAttackTriggerEvent->EventReceived.AddDynamic(this, &UGA_DK_PrimaryComboOne::OnAttackTriggered);
		WaitAttackTriggerEvent->ReadyForActivation();
	}
}

void UGA_DK_PrimaryComboOne::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HammerActor)
	{
		HammerActor->OnEndPlay.RemoveDynamic(this, &UGA_DK_PrimaryComboOne::ProjectileEndPlay);
		HammerActor->Destroy();
		HammerActor = nullptr;	
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// 투사체 생성
void UGA_DK_PrimaryComboOne::OnAttackTriggered(FGameplayEventData Payload)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !HammerActorClass)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	if (!Character->HasAuthority())
	{
		return ; // 서버에서만 실행
	}

	// TriggerEventData -> TargetData에서 목표 위치 얻기
	FVector TargetPoint = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(SavedTargetData, 0);

	// 방향 구하고 헤머 액터 생성
	// 소켓으로부터 위치를 가져와야 함.
	if (Character->GetMesh() && Character->GetMesh()->DoesSocketExist("HammerSocket"))
	{
		// 소켓 트랜스폼 구하기
		const FTransform SocketTransform = Character->GetMesh()->GetSocketTransform("HammerSocket");
		const FVector SocketLocation = SocketTransform.GetLocation();
		const FVector Direction = (TargetPoint - SocketLocation).GetSafeNormal();
		const FRotator SpawnRotation = Direction.Rotation();

		HammerActor = GetWorld()->SpawnActorDeferred<AHammerActor>(HammerActorClass, FTransform(SpawnRotation, SocketLocation), Character, Character);
		if (HammerActor)
		{
			HammerActor->OnEndPlay.AddDynamic(this, &UGA_DK_PrimaryComboOne::ProjectileEndPlay);
			UGameplayStatics::FinishSpawningActor(HammerActor, FTransform(SpawnRotation, SocketLocation));
			
			FGameplayEffectSpecHandle DamageGEHandle = MakeOutgoingGameplayEffectSpec(DamageGE, 1.0f);
			HammerActor->InitProjectile(DamageGEHandle, GetAbilitySystemComponentFromActorInfo(), FVector2D(TargetPoint.X, TargetPoint.Y), Duration);
		}
	}
}

void UGA_DK_PrimaryComboOne::ProjectileEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (HammerActor)
	{
		HammerActor->OnEndPlay.RemoveDynamic(this, &UGA_DK_PrimaryComboOne::ProjectileEndPlay);
		HammerActor = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_DK_PrimaryComboOne::ActivateTargetActor()
{
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(
		this,
		NAME_None,
		EGameplayTargetingConfirmation::Instant,
		TargetActorClass
	);
	
	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_DK_PrimaryComboOne::OnTargetDataReady);

	AGameplayAbilityTargetActor* SpawnedActor = nullptr;
	if (WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, SpawnedActor))
	{
		// 여기서 커스텀 세팅도 가능함.
		AGATA_DK_PrimaryComboOne* WaitTargetDataActor = Cast<AGATA_DK_PrimaryComboOne>(SpawnedActor);
		if (WaitTargetDataActor)
		{
			WaitTargetDataActor->RangeDiameter = TargetActorRange;
		}
		WaitTargetDataTask->FinishSpawningActor(this, SpawnedActor);
	}
	WaitTargetDataTask->ReadyForActivation();
}

void UGA_DK_PrimaryComboOne::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data)
{
	SavedTargetData = Data;
}
