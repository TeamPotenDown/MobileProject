#include "GATA_DK_PassiveTargetActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

AGATA_DK_PassiveTargetActor::AGATA_DK_PassiveTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	ShouldProduceTargetDataOnServer = false; // 서버에서 타겟 데이터를 생성하지 않음 (클라이언트만)
	bDestroyOnConfirmation = false; // Confirm되도 파괴하지 않음

	RequiredClass = ACharacter::StaticClass(); // 기본적으로 캐릭터를 대상으로 함
}

void AGATA_DK_PassiveTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability ? Ability->GetCurrentActorInfo()->AvatarActor.Get() : nullptr;
	PrimaryPC = Ability ? Ability->GetCurrentActorInfo()->PlayerController.Get() : nullptr;

	LastTarget = nullptr;

	// 처음 한번
	BroadcastIfChanged();
}

void AGATA_DK_PassiveTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BroadcastIfChanged();
}

void AGATA_DK_PassiveTargetActor::ConfirmTargetingAndContinue()
{
	if (NewTarget.Get() != LastTarget.Get())
	{
		FGameplayAbilityTargetDataHandle Handle;

		Handle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(NewTarget.Get());

		TargetDataReadyDelegate.Broadcast(Handle);
	}
	LastTarget = NewTarget;
}

void AGATA_DK_PassiveTargetActor::BroadcastIfChanged()
{
	AActor* Source = SourceActor.Get();
	if (!Source)
	{
		return ;
	}

	NewTarget = nullptr;
	TArray<AActor*> OverlappedActors;
	if (!GatherBySphere(Source, OverlappedActors))
	{
		ConfirmTargetingAndContinue();
		return ;
	}

	FVector MoveDir;
	const bool bMoving = GetMoveDir(Source, MoveDir);
	if (bMoving)
	{
		NewTarget = PickDirectionalNearest(Source, OverlappedActors, MoveDir);
	}
	// 움직였는데 방향이 달라서 위의 함수에서 아무것도 못 찾을 수도 있음. 그러니 반드시 실행
	NewTarget = PickNearest(Source, OverlappedActors);

	ConfirmTargetingAndContinue();
}

AActor* AGATA_DK_PassiveTargetActor::PickDirectionalNearest(AActor* Source, const TArray<AActor*>& OverlappedActors,
	const FVector& MoveDir)
{
	const FVector Origin = Source->GetActorLocation();

	AActor* NearestActor = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (AActor* Actor : OverlappedActors)
	{
		if (Actor || Actor->IsValidLowLevel() || Actor->IsPendingKillPending())
		{
			continue;
		}

		FVector DistanceVector = Actor->GetActorLocation() - Origin;
		const float Dist = DistanceVector.Size();
		if (Dist < KINDA_SMALL_NUMBER)
		{
			return Actor;
		}

		// 방향 벡터가 같은 방향이 아니면 무시 (어느정도 같은 방향이어야 함)
		const FVector Dir = DistanceVector.GetSafeNormal();
		const float Dot = FVector::DotProduct(Dir, MoveDir);
		if (Dot < ForwardDotMin)
		{
			continue;
		}

		// 각도에 따른 패널티 부여 : 정확히 바라보고 있는 객체에게 멀더라도 더 좋은 점수를 부여
		// 이게 싫으면 PenaltyRate를 0으로 설정하면 됨
		float AnglePenalty = (1.0f - Dot) * PenaltyRate;
		float Score = Dist + AnglePenalty;
		if (Score < BestScore)
		{
			BestScore = Score;
			NearestActor = Actor;
		}
	}
	return NearestActor;
}

AActor* AGATA_DK_PassiveTargetActor::PickNearest(AActor* Source, const TArray<AActor*>& OverlappedActors)
{
	// 이미 이전에서 찾았다면 실행 취소
	if (NewTarget.Get())
	{
		return NewTarget.Get();
	}

	// 최단거리로 선택
	const FVector Origin = Source->GetActorLocation();
	AActor* NearestActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (AActor* Actor : OverlappedActors)
	{
		if (!Actor || !Actor->IsValidLowLevel() || Actor->IsPendingKillPending())
		{
			continue;
		}

		const float Distance = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (Distance < BestDistanceSquared)
		{
			BestDistanceSquared = Distance;
			NearestActor = Actor;
		}
	}
	return NearestActor;
}

bool AGATA_DK_PassiveTargetActor::GatherBySphere(AActor* Source, TArray<AActor*>& OutOverlappedActors) const
{
	const FVector Origin = Source->GetActorLocation();

	const float Radius = FMath::Max(1.f, RangeDiameter * 0.5f);
	
	TArray<AActor*> Ignore;
	Ignore.Add(Source);
	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		Source,
		Origin,
		Radius,
		ObjectTypes,
		RequiredClass,
		Ignore,
		OutOverlappedActors
	);

	return !OutOverlappedActors.IsEmpty();
}

bool AGATA_DK_PassiveTargetActor::GetMoveDir(AActor* Source, FVector& OutDirection)
{
	APawn* Pawn = Cast<APawn>(Source);
	if (!Pawn)
	{
		return false;
	}

	const FVector LastInput = Pawn->GetLastMovementInputVector();
	if (LastInput.IsNearlyZero())
	{
		return false;
	}

	OutDirection = LastInput.GetSafeNormal();
	return true;
}
