// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ASkyeCharacter::ASkyeCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 메쉬
	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(RootComponent);
	CharacterMesh->SetRelativeLocationAndRotation(FVector(0, 0, -89), FRotator(0, -90, 0));

	// 카메라
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->SetWorldRotation(FRotator(-60, 180, 0));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);

	// 시점
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement=false;

	// Input
	ConstructorHelpers::FObjectFinder<UInputMappingContext> SkyeIMC(TEXT("/Game/Skye/Input/IMC_Skye.IMC_Skye"));
	if (SkyeIMC.Succeeded())
	{
		DefaultMappingContext = SkyeIMC.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> Skye_Move(TEXT("/Game/Skye/Input/IA_SkyeMoveMobile.IA_SkyeMoveMobile"));
	if (Skye_Move.Succeeded())
	{
		MoveAction = Skye_Move.Object;
	}
}

class UAbilitySystemComponent* ASkyeCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void ASkyeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		for (const auto& it : StartAbilities)
		{
			FGameplayAbilitySpec StartSpec(it);
			ASC->GiveAbility(StartSpec);
		}
	}
	SetOwner(NewController);
}

// Called when the game starts or when spawned
void ASkyeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC{Cast<APlayerController>(Controller)})
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem
			{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())
		})
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called every frame
void ASkyeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASkyeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent) return;
	
	if (UEnhancedInputComponent* EnhancedInputComponent{Cast<UEnhancedInputComponent>(PlayerInputComponent)})
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkyeCharacter::Move);
	}
}

void ASkyeCharacter::Move(const struct FInputActionValue& Value)
{
	if (!Controller)  return;
	
	FVector2D MoveInput = Value.Get<FVector2D>();
	
	if (Controller && (MoveInput.SizeSquared() > 0.0f))
	{
		// 1. 카메라 기준 방향 가져오기
		const FRotator CameraRot = GetControlRotation();
		const FVector Forward = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);

		// 2. 입력 벡터 → 월드 방향 변환
		FVector MoveDir = (Forward * MoveInput.Y + Right * MoveInput.X);
		MoveDir.Z = 0.0f;
		MoveDir.Normalize();

		// 3. 이동
		AddMovementInput(MoveDir);

		// 4. 캐릭터 회전 수동 처리
		FRotator TargetRot = MoveDir.Rotation();
		SetActorRotation(TargetRot);
	}
}