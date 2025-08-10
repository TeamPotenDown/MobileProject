// CRAttributeSet.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CRAttributeSet.generated.h"

// 속성 접근자 매크로
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MOBILEPROJECT_API UCRAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UCRAttributeSet();

    // AttributeSet 복제
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 속성 변경 전 처리
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    
    // GameplayEffect 적용 후 처리
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // === 기본 속성 ===
    
    // 체력
    UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, MaxHealth)

    // 마나/에너지
    UPROPERTY(BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_Mana)
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, Mana)

    UPROPERTY(BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_MaxMana)
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, MaxMana)

    // 이동 속도
    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MoveSpeed)
    FGameplayAttributeData MoveSpeed;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, MoveSpeed)

    // 공격력
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackDamage)
    FGameplayAttributeData AttackDamage;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, AttackDamage)

    // 공격 속도
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackSpeed)
    FGameplayAttributeData AttackSpeed;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, AttackSpeed)

    // 공격 사거리
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackRange)
    FGameplayAttributeData AttackRange;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, AttackRange)

    // === 추가 속성 ===
    
    // 크리티컬 확률 (0-100)
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CriticalChance)
    FGameplayAttributeData CriticalChance;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, CriticalChance)

    // 크리티컬 데미지 배율
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CriticalDamage)
    FGameplayAttributeData CriticalDamage;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, CriticalDamage)

    // 쿨다운 감소 (0-100%)
    UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CooldownReduction)
    FGameplayAttributeData CooldownReduction;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, CooldownReduction)

    // 방어력/데미지 감소
    UPROPERTY(BlueprintReadOnly, Category = "Defense", ReplicatedUsing = OnRep_Armor)
    FGameplayAttributeData Armor;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, Armor)

    // 체력 재생
    UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthRegen)
    FGameplayAttributeData HealthRegen;
    ATTRIBUTE_ACCESSORS(UCRAttributeSet, HealthRegen)

protected:
    // RepNotify 함수들
    UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

    UFUNCTION()
    virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

    UFUNCTION()
    virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

    UFUNCTION()
    virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

    UFUNCTION()
    virtual void OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage);

    UFUNCTION()
    virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);

    UFUNCTION()
    virtual void OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange);

    UFUNCTION()
    virtual void OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance);

    UFUNCTION()
    virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage);

    UFUNCTION()
    virtual void OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction);

    UFUNCTION()
    virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

    UFUNCTION()
    virtual void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen);
};