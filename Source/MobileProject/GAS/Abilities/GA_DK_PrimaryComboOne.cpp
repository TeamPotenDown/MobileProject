#include "GA_DK_PrimaryComboOne.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MobileProject/Character/Player/DKCharacter/Hammer/HammerActor.h"
#include "MobileProject/Utils/LogUtils.h"

UGA_DK_PrimaryComboOne::UGA_DK_PrimaryComboOne()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; // 서버에서만 실행되도록 설정

	// 트리거는 에디터에서 설정하자. // Event.Deulee.PrimaryAttack1 설정
}

void UGA_DK_PrimaryComboOne::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	MP_LOGF(LogMP, Log, TEXT("GA_DK_PrimaryComboOne::ActivateAbility - Starting ability with Handle: %s"), *Handle.ToString());

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !HammerActorClass)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// TriggerEventData -> TargetData에서 목표 위치 얻기
	FVector TargetPoint = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(TriggerEventData ? TriggerEventData->TargetData : FGameplayAbilityTargetDataHandle(), 0);

	// 방향 구하고 헤머 액터 생성
	// 소켓으로부터 위치를 가져와야 함.
	if (Character->GetMesh() && Character->GetMesh()->DoesSocketExist("HammerSocket"))
	{
		MP_LOGF(LogMP, Log, TEXT("GA_DK_PrimaryComboOne::ActivateAbility - Spawning HammerActor at %s"), *TargetPoint.ToString());
		// 소켓 트랜스폼 구하기
		const FTransform SocketTransform = Character->GetMesh()->GetSocketTransform("HammerSocket");
		const FVector SocketLocation = SocketTransform.GetLocation();
		const FVector Direction = (TargetPoint - SocketLocation).GetSafeNormal();
		const FRotator SpawnRotation = Direction.Rotation();

		HammerActor = GetWorld()->SpawnActorDeferred<AHammerActor>(HammerActorClass, FTransform(SpawnRotation, SocketLocation), Character, Character);
		if (HammerActor)
		{
			// 해머 액터가 생성되었으면, Duration 후에 종료되도록 설정
			HammerActor->SetLifeSpan(Duration);
			HammerActor->OnEndPlay.AddDynamic(this, &UGA_DK_PrimaryComboOne::ProjectileEndPlay);
		}
		UGameplayStatics::FinishSpawningActor(HammerActor, FTransform(SpawnRotation, SocketLocation));
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

void UGA_DK_PrimaryComboOne::ProjectileEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (HammerActor)
	{
		HammerActor->OnEndPlay.RemoveDynamic(this, &UGA_DK_PrimaryComboOne::ProjectileEndPlay);
		HammerActor = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
