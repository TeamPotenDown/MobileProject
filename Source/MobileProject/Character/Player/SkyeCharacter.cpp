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
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -89), FRotator(0, -90, 0));
	
	// 카메라
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->SetWorldRotation(FRotator(-60, 0, 0));
	SpringArmComp->TargetArmLength = 600.f;
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
	ConstructorHelpers::FObjectFinder<UInputAction> Skye_NormalAttack(TEXT("/Game/Skye/Input/IA_SkyeNormalAttack.IA_SkyeNormalAttack"));
	if (Skye_NormalAttack.Succeeded())
	{
		NormalAttackAction = Skye_NormalAttack.Object;
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

		int32 InputId = 0;
		for (const auto& it : StartAbilities)
		{
			FGameplayAbilitySpec StartSpec(it);
			StartSpec.InputID = InputId++;
			ASC->GiveAbility(StartSpec);
			SetupGASPlayerInputComponent();
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

	SetupGASPlayerInputComponent();
	
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
		// 카메라 기준 방향 가져오기
		const FRotator CameraRot = GetControlRotation();
		const FVector Forward = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);

		// 입력 벡터를 월드 방향 변환
		FVector MoveDir = (Forward * MoveInput.Y + Right * MoveInput.X);
		MoveDir.Z = 0.0f;
		MoveDir.Normalize();

		// 이동, 캐릭터 회전 수동 처리
		AddMovementInput(MoveDir);
		FRotator TargetRot = MoveDir.Rotation();
		SetActorRotation(TargetRot);
	}
}

void ASkyeCharacter::SetupGASPlayerInputComponent()
{
	if (IsValid(ASC) && InputComponent)
	{
		UEnhancedInputComponent* EnhancedInputComponent{Cast<UEnhancedInputComponent>(InputComponent)};

		EnhancedInputComponent->BindAction(NormalAttackAction, ETriggerEvent::Triggered, this, &ASkyeCharacter::GASInputPressed, 0);
	}
}

void ASkyeCharacter::GASInputPressed(int32 InputId)
{
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId);
	if (Spec)
	{
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			ASC->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			ASC->TryActivateAbility(Spec->Handle);
		}
	}
}

void ASkyeCharacter::GASInputReleased(int32 InputId)
{
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId);
	if (Spec)
	{
		Spec->InputPressed = false;
		if (Spec->IsActive())
		{
			ASC->AbilitySpecInputReleased(*Spec);
		}
	}
}
