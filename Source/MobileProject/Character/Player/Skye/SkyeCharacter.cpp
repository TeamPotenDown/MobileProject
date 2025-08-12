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
	SpringArmComp->SetUsingAbsoluteRotation(true);
	SpringArmComp->bUsePawnControlRotation = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 시점
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

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
	ConstructorHelpers::FObjectFinder<UInputAction> Skye_ChargeAttack(TEXT("/Game/Skye/Input/IA_SkyeChargeAttack.IA_SkyeChargeAttack"));
	if (Skye_ChargeAttack.Succeeded())
	{
		ChargeAttackAction = Skye_ChargeAttack.Object;
	}
}

class UAbilitySystemComponent* ASkyeCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void ASkyeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	SetOwner(NewController);
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		for (const auto& Pair : StartInputAbilities)
		{
			const ESkyeAbilityEnum InputEnum = Pair.Key;
			TSubclassOf<UGameplayAbility> AbilityClass = Pair.Value;

			if (InputEnum == ESkyeAbilityEnum::None || !AbilityClass) continue;
			
			FGameplayAbilitySpec Spec(AbilityClass);
			Spec.InputID = static_cast<int32>(InputEnum); // enum -> InputID
			ASC->GiveAbility(Spec);
		}

		SetupGASPlayerInputComponent();
		APlayerController* PC = CastChecked<APlayerController>(NewController);
		PC->ConsoleCommand(TEXT("showdebug abilitysystem"));
	}
}

void ASkyeCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
		SetupGASPlayerInputComponent();
	}
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
	if (MoveInput.IsNearlyZero()) return;

	// 카메라의 'Yaw'만 사용해서 평면 기준 축 계산
	FRotator CamYawRot(0.f, 0.f, 0.f);

	// 우선순위: 내 탑다운 카메라 컴포넌트 > 플레이어 카메라 매니저 > 컨트롤러
	if (CameraComp)
	{
		CamYawRot.Yaw = CameraComp->GetComponentRotation().Yaw;
	}
	else if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		CamYawRot.Yaw = PC->PlayerCameraManager->GetCameraRotation().Yaw;
	}
	else
	{
		CamYawRot.Yaw = Controller->GetControlRotation().Yaw;
	}
	const FVector CamForward = FRotationMatrix(CamYawRot).GetUnitAxis(EAxis::X); // 화면 위쪽
	const FVector CamRight   = FRotationMatrix(CamYawRot).GetUnitAxis(EAxis::Y); // 화면 오른쪽

	// 축별로 넣기 위(+Y)=카메라 앞, 아래(-Y)=카메라 쪽
	AddMovementInput(CamForward, MoveInput.Y);
	AddMovementInput(CamRight, MoveInput.X);
}

void ASkyeCharacter::SetupGASPlayerInputComponent()
{
	if (IsValid(ASC) && InputComponent)
	{
		UEnhancedInputComponent* EIC{Cast<UEnhancedInputComponent>(InputComponent)};

		EIC->BindAction(NormalAttackAction, ETriggerEvent::Triggered, this,
			&ASkyeCharacter::GASInputPressed, ESkyeAbilityEnum::ComboAttack);
		EIC->BindAction(ChargeAttackAction, ETriggerEvent::Triggered, this,
			&ASkyeCharacter::GASInputPressed, ESkyeAbilityEnum::ChargeAttack);
		EIC->BindAction(ChargeAttackAction, ETriggerEvent::Completed, this,
			&ASkyeCharacter::GASInputReleased, ESkyeAbilityEnum::ChargeAttack);
	}
}

void ASkyeCharacter::GASInputPressed(ESkyeAbilityEnum InputId)
{
	if (!ASC) return;

	/*
	 *InputID 기반으로 충분하고, 단일 능력만 연결된 단순 케이스 → 그냥 ASC->AbilityLocalInputPressed(InputID) / AbilityLocalInputReleased(InputID) 써도 됨.
	 *TODO: 단, bReplicateInputDirectly = false(기본값 유지)로 두고, 능력 내부에서 WaitInputPress/Release 태스크로 처리하면 네트워크적으로도 안전하다.
	*/
	ASC->AbilityLocalInputPressed(static_cast<int32>(InputId));
}

void ASkyeCharacter::GASInputReleased(ESkyeAbilityEnum InputId)
{
	if (!ASC) return;

	ASC->AbilityLocalInputReleased(static_cast<int32>(InputId));
}
