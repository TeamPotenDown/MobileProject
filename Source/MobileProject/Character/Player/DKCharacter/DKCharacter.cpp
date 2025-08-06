#include "DKCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ADKCharacter::ADKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 800.0f;
	// topdown view : 독립적으로 회전
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->SetWorldRotation({-60.0f, 0.0f, 0.0f});
	FRotator SpringArmRotation = FRotator(0.0f, 0.0f, 0.0f);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ADKCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ADKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!NewController->IsLocalController())
	{
		return ;
		
	}
	APlayerController* PC = Cast<APlayerController>(NewController);
	check(PC);
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputContext, 0);
	}
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

	if (!PlayerInputComponent)
	{
		return ;
	}
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADKCharacter::HandleMove);
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