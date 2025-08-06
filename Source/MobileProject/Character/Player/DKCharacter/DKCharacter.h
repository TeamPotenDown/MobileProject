#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "DKCharacter.generated.h"

UCLASS()
class MOBILEPROJECT_API ADKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADKCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Input|Movement")
	void Move(FVector2D Direction);

protected:
	UFUNCTION()
	void HandleMove(const FInputActionValue& InputActionValue);
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement|Context", meta = (DisplayName = "Movement Context"), meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputMappingContext> InputContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement|Action", meta = (DisplayName = "Move Action"), meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> CameraComponent;
    UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArmComponent;
};
