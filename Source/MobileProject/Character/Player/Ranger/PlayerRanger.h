#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PlayerRanger.generated.h"

class UAbilitySystemComponent;
class UCRAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class MOBILEPROJECT_API APlayerRanger : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    APlayerRanger();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

public:
    virtual void Tick(float DeltaTime) override;

    // AbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // 초기화
    void InitializeAttributes();
    void GiveDefaultAbilities();

    // 입력 바인딩
    void BindASCInput();

    // === 전투 ===
    void PerformAutoAttack();
    AActor* FindNearestTarget();
    FVector GetAttackDirection();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TestStartAutoAttack() {StartAutoAttack();};
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TestStopAutoAttack() {StopAutoAttack();};
protected:
    // === Enhanced Input ===
    void Move(const FInputActionValue& Value);
    void StartAiming();
    void StopAiming();
    void StartAutoAttack();
    void StopAutoAttack();

    // === 이동 및 회전 ===
    void UpdateCharacterRotation();
    void RotateToMovementDirection();
    void RotateToAimDirection();



    // === 모바일 입력 처리 ===
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetJoystickInput(FVector2D JoystickValue);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetAimInput(FVector2D AimValue);

protected:
    // === Components ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UCRAttributeSet* AttributeSet;

    // === Enhanced Input ===
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* AimAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* AttackAction;

    // === GAS Configuration ===
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayEffect>> DefaultAttributeEffects;

    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    // 평타 어빌리티
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> BasicAttackAbilityClass;

    // === 입력 상태 ===
    FVector2D MovementInput;
    FVector2D AimInput;
    bool bIsAiming = false;
    bool bIsAutoAttacking = false;

    // 자동 공격 타이머
    FTimerHandle AutoAttackTimerHandle;
    float LastAttackTime = 0.0f;

    // === 전투 설정 ===
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float AutoTargetRange = 800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RotationSpeed = 10.0f;

    // 현재 타겟
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

public:
    AActor* GetCurrentTarget() const;

protected:
    // ASC 입력 바인딩 여부
    bool bASCInputBound = false;

    // 어빌리티 입력 ID
    int32 BasicAttackInputID = 0;
};