#include "DKCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MobileProject/GAS/Abilities/GA_DK_PrimaryAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Component/DKAbilitySystemComponent.h"
#include "Input/EAbilityInputID.h"
#include "MobileProject/GAS/Tags/GamePlayTagsUtils.h"
#include "MobileProject/Utils/LogUtils.h"

ADKCharacter::ADKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/*
	 *********************
	 * CAMERA SETTING
	 ******************** 
	*/
	static constexpr float DEFAULT_ARM_LENGTH = 800.0f;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = DEFAULT_ARM_LENGTH;
	// topdown view : 독립적으로 회전
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->SetWorldRotation({-60.0f, 0.0f, 0.0f});
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	/*
	 *****************
	 * GAS SETTING
	 *****************
	*/
	ASC = CreateDefaultSubobject<UDKAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void ADKCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// PossessedBy는 서버에서만 호출됨
void ADKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitASC();
	if (HasAuthority())
	{
		GrantStartupAbilities();
	}
	SetOwner(NewController);
}

// Client에서 호출
void ADKCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitASC();
}

void ADKCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input 컨텍스트는 로컬 컨트롤러에서 추가
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputContext)
				{
					Subsystem->AddMappingContext(InputContext, /*Priority=*/0);
				}
			}
		}
	}
	
	if (IsLocallyControlled() && ASC && !bASCInputBound)
	{
		BindASCInput();
	}
}

void ADKCharacter::HandleMove(const FInputActionValue& InputActionValue)
{
	const FVector2D Direction = InputActionValue.Get<FVector2D>();
	
	Move(Direction);
}

void ADKCharacter::Move(FVector2D Direction)
{
	if (!Controller)
	{
		return ;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();

	const FRotator YawRotation(0, ControlRotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector MoveDirection = (ForwardDirection * Direction.Y) + (RightDirection * Direction.X);
	
	// ScaleValue는 1.0f로 고정되어 있지만, AttributeSet으로 동적으로 조정하자!
	AddMovementInput(MoveDirection, 1.0f);
}

void ADKCharacter::InitASC()
{
	ASC->InitAbilityActorInfo(this, this);
	// 입력 바인딩은 오너만
	if (IsLocallyControlled() && !bASCInputBound && InputComponent)
	{
		BindASCInput();

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC) PC->ConsoleCommand(TEXT("showdebug abilitysystem"));
	}
}

void ADKCharacter::GrantStartupAbilities()
{
	if (!HasAuthority() || !ASC)
	{
		return;
	}

	for (const TSubclassOf<UGA_DK_GameplayAbilityBase>& GAClass : StartupAbilities)
	{
		if (!*GAClass)
		{
			continue;
		}
		
		const UGA_DK_GameplayAbilityBase* CDO = GAClass->GetDefaultObject<UGA_DK_GameplayAbilityBase>();

		FGameplayAbilitySpec Spec(GAClass, 1,
			static_cast<int32>(CDO->GetInputID()), this);

		ASC->GiveAbility(Spec);
	}
}

void ADKCharacter::BindASCInput()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADKCharacter::HandleMove);
		
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &ADKCharacter::OnInputPressed, EAbilityInputID::Ability1);
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &ADKCharacter::OnInputReleased, EAbilityInputID::Ability1);
	}
	
	bASCInputBound = true;
}

void ADKCharacter::OnInputPressed(const EAbilityInputID ID)
{
	ASC->AbilityLocalInputPressed(static_cast<int32>(ID));
}

void ADKCharacter::OnInputReleased(const EAbilityInputID ID)
{
	ASC->AbilityLocalInputReleased(static_cast<int32>(ID));
}
