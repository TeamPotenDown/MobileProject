// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Move(const struct FInputActionValue& Value);
	void Attack(const struct FInputActionValue& Value);

protected:
	// GAS 관련 input 바인딩 처리 함수
	UFUNCTION()
	void SetupGASPlayerInputComponent();
	UFUNCTION()
	void GASInputPressed(int32 InputId);
	UFUNCTION()
	void GASInputReleased(int32 InputId);
	
public:
	// 카메라
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* CameraComp;

	// AbilitySystemComponent
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> StartAbilities;

	// combo montage
	virtual class UAnimMontage* GetComboActionMontage() const{ return ComboActionMontage; };
protected:
	// anim montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* ComboActionMontage;

protected:
	// input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* MoveAction;
	
	// GAS input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* NormalAttackAction;
};
