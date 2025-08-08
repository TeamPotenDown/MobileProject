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
#include "MobileProject/GAS/Abilities/GA_DKAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
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
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Full);
}

void ADKCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (!PC->IsLocalController())
		{
			MP_LOGF(LogMP, Error, TEXT("ADKCharacter::BeginPlay - Not Local Controller!"));
			return;
		}
		
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(InputContext, 0);
			}
		}
	}
}

// PossessedBy는 서버에서만 호출됨
void ADKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	MP_LOGF(LogMP, Warning, TEXT("ADKCharacter::PossessedBy called"));
	
	APlayerController* PC = Cast<APlayerController>(NewController);
	if (!PC)
	{
		MP_LOGF(LogMP, Error, TEXT("PlayerController is null!"));
		return;
	}

	ASC->InitAbilityActorInfo(this, this);

	for (const TPair<int32, TSubclassOf<UGameplayAbility>>& Pair : DefaultAbilities)
	{
		FGameplayAbilitySpec AbilitySpec(Pair.Value);
		AbilitySpec.InputID = Pair.Key;
		UE_LOG(LogTemp, Log, TEXT("ADKCharacter::PossessedBy - GiveAbility: %s"), *Pair.Value.GetDefaultObject()->GetName());
		ASC->GiveAbility(AbilitySpec);
	}
	PC->ConsoleCommand(TEXT("showdebug abilitysystem"));

	SetOwner(NewController);
}

// Client에서 호출
void ADKCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	MP_LOGF(LogMP, Warning, TEXT("ADKCharacter::OnRep_PlayerState called"));
	MP_LOGF(LogMP, Warning, TEXT("Am I Client? %s"), (IsLocallyControlled() && !HasAuthority()) ? TEXT("Yes") : TEXT("No"));
	
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
	{
		MP_LOGF(LogMP, Error, TEXT("PlayerController is null!"));
		return;
	}

	if (!PC->IsLocalController() || PC->HasAuthority())
	{
		MP_LOGF(LogMP, Warning, TEXT("ADKCharacter::OnRep_PlayerState - Not Local Controller or Server"));
		return ;
	}
	
	ASC->InitAbilityActorInfo(this, this);

	PC->ConsoleCommand(TEXT("showdebug abilitysystem"));
}

// Called every frame
void ADKCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent || !IsLocallyControlled())
	{
		return ;
	}
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADKCharacter::HandleMove);
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &ADKCharacter::OnPrimaryAttackPressed);
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

void ADKCharacter::OnPrimaryAttackPressed()
{
	// TODO: 하드코딩 없애기
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(1);
	
	if (!Spec)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_DKAttack not found in Ability System Component"));
		return;
	}

	const bool bAbilityActive =
		Spec && Spec->IsActive();

	Spec->InputPressed = true;
	if (bAbilityActive)
	{
		if (Spec->Ability->bReplicateInputDirectly && ASC->IsOwnerActorAuthoritative() == false)
		{
			ASC->ServerSetInputPressed(Spec->Handle);
		}
		ASC->AbilitySpecInputPressed(*Spec);
	}
	else
	{
		ASC->TryActivateAbility(Spec->Handle);
	}
}
