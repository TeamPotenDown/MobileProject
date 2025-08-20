// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ESkyeAbilityEnum.h"
#include "SkyeCharacter.generated.h"

UCLASS()
class MOBILEPROJECT_API ASkyeCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASkyeCharacter();

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Move(const struct FInputActionValue& Value);

protected:
	// GAS 관련 input 바인딩 처리 함수
	UFUNCTION()
	void SetupGASPlayerInputComponent();
	UFUNCTION()
	void GASInputPressed(ESkyeAbilityEnum InputId);
	UFUNCTION()
	void GASInputReleased(ESkyeAbilityEnum InputId);
	
	// input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* MoveAction;
	
	// GAS input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* NormalAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* ChargeAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* DashAction;
	
public:
	// 카메라
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* CameraComp;

	// GAS | AbilitySystemComponent
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
	// 시작 시 부여할 Ability 목록
	UPROPERTY(EditAnywhere, Category = GAS)
	TMap<ESkyeAbilityEnum, TSubclassOf<class UGameplayAbility>> StartInputAbilities;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "GAS")
	TObjectPtr<class UAttributeSet> AttributeSet; // 추후구현
};
