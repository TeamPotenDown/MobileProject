#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Input/EAbilityInputID.h"
#include "DKCharacter.generated.h"

UCLASS()
class MOBILEPROJECT_API ADKCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADKCharacter();

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Input|Movement")
	void Move(FVector2D Direction);
	UFUNCTION(BlueprintCallable, Category = "Input|Attack")
	void OnInputPressed(const EAbilityInputID InputID);
	UFUNCTION(BlueprintCallable, Category = "Input|Attack")
	void OnInputReleased(const EAbilityInputID InputID);

protected:
	
	void InitASC();
	void GrantStartupAbilities();
	void BindASCInput();
	
	bool bASCInputBound = false;
	
protected:
	UFUNCTION()
	void HandleMove(const FInputActionValue& InputActionValue);
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement|Context", meta = (DisplayName = "Movement Context"), meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputMappingContext> InputContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (DisplayName = "Move Action"), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement|Action", meta = (DisplayName = "Move Action"), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> PrimaryAttackAction;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> CameraComponent;
    UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class UGA_DK_GameplayAbilityBase>> StartupAbilities;
};
