// CRAttributeSet.cpp
#include "CRAttributeSet.h"

#include <string>

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UCRAttributeSet::UCRAttributeSet()
{
    // 기본값 설정
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitMana(100.0f);
    InitMaxMana(100.0f);
    InitMoveSpeed(600.0f);
    InitAttackDamage(20.0f);
    InitAttackSpeed(1.0f);  // 초당 공격 횟수
    InitAttackRange(800.0f);  // 언리얼 단위
    InitCriticalChance(10.0f);  // 10%
    InitCriticalDamage(2.0f);  // 200% 데미지
    InitCooldownReduction(0.0f);
    InitArmor(0.0f);
    InitHealthRegen(1.0f);  // 초당 회복량
}

void UCRAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, CooldownReduction, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, Armor, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UCRAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
}

void UCRAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 속성 값 제한
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
    }
    else if (Attribute == GetMoveSpeedAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
    else if (Attribute == GetAttackSpeedAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.1f);  // 최소 공격속도
    }
    else if (Attribute == GetCriticalChanceAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
    }
    else if (Attribute == GetCooldownReductionAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 75.0f);  // 최대 75% 쿨감
    }
}

void UCRAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer& TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();

    float DeltaValue = 0;
    if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
    {
        DeltaValue = Data.EvaluatedData.Magnitude;
    }

    AActor* TargetActor = nullptr;
    AController* TargetController = nullptr;
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
    }

    // Health 변경 처리
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

        // ToDo Delete
        if (TargetActor && DeltaValue != 0.f && GEngine)
        {
            float Current = GetHealth();
            float Max = GetMaxHealth();
        
            FString Msg = FString::Printf(
                TEXT("[%s] 체력 변화: %+0.f (현재: %.0f / 최대: %.0f)"),
                *TargetActor->GetName(),
                DeltaValue,
                Current,
                Max
            );

            FColor MsgColor = (DeltaValue > 0) ? FColor::Green : FColor::Red;

            GEngine->AddOnScreenDebugMessage(
                -1,           // Key (-1 = 중복 허용)
                2.0f,         // Duration
                MsgColor,     // Color
                Msg           // Message
            );
        }
    }
    // Mana 변경 처리
    else if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
    }
}

// RepNotify 함수 구현
void UCRAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, Health, OldHealth);
}

void UCRAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, MaxHealth, OldMaxHealth);
}

void UCRAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, Mana, OldMana);
}

void UCRAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, MaxMana, OldMaxMana);
}

void UCRAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UCRAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, AttackDamage, OldAttackDamage);
}

void UCRAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UCRAttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, AttackRange, OldAttackRange);
}

void UCRAttributeSet::OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, CriticalChance, OldCriticalChance);
}

void UCRAttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, CriticalDamage, OldCriticalDamage);
}

void UCRAttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, CooldownReduction, OldCooldownReduction);
}

void UCRAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, Armor, OldArmor);
}

void UCRAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UCRAttributeSet, HealthRegen, OldHealthRegen);
}