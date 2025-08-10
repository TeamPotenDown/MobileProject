#include "GATA_DK_PrimaryComboOne.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MobileProject/Utils/LogUtils.h"

AGATA_DK_PrimaryComboOne::AGATA_DK_PrimaryComboOne()
{
	// 클라 예측 (즉, 클라에서만 실행됨)
	// 서버는 결과값만 받음
	bReplicates = false;
	ShouldProduceTargetDataOnServer = false;

	RequiredClass = ACharacter::StaticClass(); // 기본적으로 캐릭터를 대상으로 함
}

void AGATA_DK_PrimaryComboOne::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability ? Ability->GetCurrentActorInfo()->AvatarActor.Get() : nullptr;
}

void AGATA_DK_PrimaryComboOne::ConfirmTargetingAndContinue()
{
	FGameplayAbilityTargetDataHandle Handle;

	AActor* Nearest = nullptr;
	FVector Origin, Forward;
	const bool bHasNearest = FindNearestActorInRadius(Nearest, Origin, Forward);

	if (bHasNearest)
	{
		// 배우 타깃 데이터(배열 버전 사용)
		TArray<AActor*> Arr;
		Arr.Add(Nearest);
		Handle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(Arr, true);
	}
	else
	{
		// 전방 폴백: 지름만큼 앞 지점
		const FVector FallbackPoint = Origin + Forward * RangeDiameter;

		FGameplayAbilityTargetingLocationInfo Src, Dst;
		Src.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
		Src.SourceActor = SourceActor;

		Dst.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		Dst.LiteralTransform = FTransform(FallbackPoint);

		Handle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromLocations(Src, Dst);

		MP_LOGF(LogMP, Warning, TEXT("No valid target found in radius. Using fallback point at %s"), *FallbackPoint.ToString());
	}

	TargetDataReadyDelegate.Broadcast(Handle);
}

bool AGATA_DK_PrimaryComboOne::FindNearestActorInRadius(AActor*& OutNearest, FVector& OutOrigin,
	FVector& OutForward) const
{
	OutNearest = nullptr;

	if (!SourceActor)
		return false;

	OutOrigin = SourceActor->GetActorLocation();
	OutForward = SourceActor->GetActorForwardVector();

	const float Radius = FMath::Max(1.f, RangeDiameter * 0.5f);

	// 겹침으로 후보 수집
	TArray<AActor*> Ignore;
	Ignore.Add(SourceActor);
	TArray<AActor*> Overlapped;
	UKismetSystemLibrary::SphereOverlapActors(
		SourceActor,
		OutOrigin,
		Radius,
		ObjectTypes,  
		RequiredClass,
		Ignore,
		Overlapped
	);

	// 최단거리 선택
	double BestDistSq = TNumericLimits<double>::Max();
	for (AActor* Actor : Overlapped)
	{
		if (!IsValid(Actor) || Actor == SourceActor)
		{
			continue;
		}
		
		const double DistanceSquared = FVector::DistSquared(OutOrigin, Actor->GetActorLocation());
		if (DistanceSquared < BestDistSq)
		{
			BestDistSq = DistanceSquared;
			OutNearest = Actor;
		}
	}

	return OutNearest != nullptr;
}

