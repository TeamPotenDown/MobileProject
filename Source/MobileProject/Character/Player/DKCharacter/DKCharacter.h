#pragma once

#include "CoreMinimal.h"
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

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
