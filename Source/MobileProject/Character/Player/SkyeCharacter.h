// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyeCharacter.generated.h"

UCLASS()
class MOBILEPROJECT_API ASkyeCharacter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASkyeCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
