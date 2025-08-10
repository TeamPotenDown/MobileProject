#include "PlayerRanger.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "MobileProject/GAS/Components/CRAttributeSet.h"

APlayerRanger::APlayerRanger()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 회전 설정 (이동 방향으로 회전하지 않음)
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // 캡슐 컴포넌트 설정
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // 스프링암 컴포넌트 (탑다운 뷰)
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f)); // 탑다운 각도
    SpringArmComponent->TargetArmLength = 1200.0f;
    SpringArmComponent->bDoCollisionTest = false;
    SpringArmComponent->bInheritPitch = false;
    SpringArmComponent->bInheritRoll = false;
    SpringArmComponent->bInheritYaw = false;

    // 카메라 컴포넌트
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);

    // Ability System Component
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    // Attribute Set
    AttributeSet = CreateDefaultSubobject<UCRAttributeSet>(TEXT("AttributeSet"));
}

void APlayerRanger::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 설정
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void APlayerRanger::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // 이동
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerRanger::Move);
        
        // 조준
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerRanger::StartAiming);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerRanger::StopAiming);
        
        // 공격
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerRanger::StartAutoAttack);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &APlayerRanger::StopAutoAttack);
    }

    // ASC 입력 바인딩
    //BindASCInput();
}

void APlayerRanger::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 서버에서 ASC 초기화
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        InitializeAttributes();
        GiveDefaultAbilities();
    }
}

void APlayerRanger::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // 클라이언트에서 ASC 초기화
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        InitializeAttributes();
        BindASCInput();
    }
}

void APlayerRanger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 캐릭터 회전 업데이트
    UpdateCharacterRotation();

    // 타겟 유효성 검사
    if (CurrentTarget && !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
    }
}

UAbilitySystemComponent* APlayerRanger::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void APlayerRanger::InitializeAttributes()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // 기본 속성 효과 적용
    for (TSubclassOf<UGameplayEffect>& GameplayEffect : DefaultAttributeEffects)
    {
        FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
        
        if (SpecHandle.IsValid())
        {
            FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }
}

void APlayerRanger::GiveDefaultAbilities()
{
    if (!HasAuthority() || !AbilitySystemComponent)
    {
        return;
    }

    // 기본 어빌리티 부여
    for (TSubclassOf<UGameplayAbility>& Ability : DefaultAbilities)
    {
        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this));
    }

    // 평타 어빌리티 부여
    if (BasicAttackAbilityClass)
    {
        FGameplayAbilitySpec AbilitySpec(BasicAttackAbilityClass, 1, BasicAttackInputID, this);
        AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
}

void APlayerRanger::BindASCInput()
{
    if (!bASCInputBound && AbilitySystemComponent && InputComponent)
    {
        AbilitySystemComponent->BindAbilityActivationToInputComponent(
            InputComponent,
            FGameplayAbilityInputBinds(
                FString("ConfirmTarget"),
                FString("CancelTarget"),
                FString("EGASAbilityInputID"),
                static_cast<int32>(BasicAttackInputID),
                static_cast<int32>(BasicAttackInputID)
            )
        );

        bASCInputBound = true;
    }
}

void APlayerRanger::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    MovementInput = MovementVector;

    if (Controller != nullptr)
    {
        // 카메라 기준으로 이동 방향 계산
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw -180, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void APlayerRanger::StartAiming()
{
    bIsAiming = true;
}

void APlayerRanger::StopAiming()
{
    bIsAiming = false;
}

void APlayerRanger::StartAutoAttack()
{
    bIsAutoAttacking = true;
    
    // 즉시 첫 공격 수행
    PerformAutoAttack();
    
    // 공격 속도에 따른 자동 공격 타이머 설정
    if (AttributeSet)
    {
        float AttackSpeed = AttributeSet->GetAttackSpeed();
        float AttackInterval = 1.0f / AttackSpeed;
        
        GetWorldTimerManager().SetTimer(
            AutoAttackTimerHandle,
            this,
            &APlayerRanger::PerformAutoAttack,
            AttackInterval,
            true,
            AttackInterval
        );
    }
}

void APlayerRanger::StopAutoAttack()
{
    bIsAutoAttacking = false;
    GetWorldTimerManager().ClearTimer(AutoAttackTimerHandle);
}

void APlayerRanger::UpdateCharacterRotation()
{
    if (bIsAiming && AimInput.Size() > 0.1f)
    {
        RotateToAimDirection();
    }
    else if (MovementInput.Size() > 0.1f)
    {
        RotateToMovementDirection();
    }
}

void APlayerRanger::RotateToMovementDirection()
{
    // 카메라 기준 월드 방향으로 변환
    if (Controller != nullptr && MovementInput.Size() > 0.1f)
    {
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw - 180, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        // 입력을 월드 방향으로 변환
        FVector WorldDirection = ForwardDirection * MovementInput.Y + RightDirection * MovementInput.X;
        
        FRotator TargetRotation = WorldDirection.Rotation();
        TargetRotation.Pitch = 0.0f;
        TargetRotation.Roll = 0.0f;
        
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, 
                                                GetWorld()->GetDeltaSeconds(), RotationSpeed);
        SetActorRotation(NewRotation);
    }
}

void APlayerRanger::RotateToAimDirection()
{
    // 조준 방향으로 회전
    FVector AimDirection = FVector(AimInput.X, AimInput.Y, 0.0f);
    
    if (AimDirection.Size() > 0.1f)
    {
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = AimDirection.Rotation();
        TargetRotation.Pitch = 0.0f;
        TargetRotation.Roll = 0.0f;
        
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed * 1.5f);
        SetActorRotation(NewRotation);
    }
}

void APlayerRanger::PerformAutoAttack()
{
    if (!AbilitySystemComponent || !BasicAttackAbilityClass)
    {
        return;
    }

    // 타겟 찾기
    CurrentTarget = FindNearestTarget();
    if (nullptr != CurrentTarget)
    {
        DrawDebugSphere(GetWorld(), CurrentTarget->GetActorLocation() + FVector(0,0,80.f), 30.f, 10, FColor::Red, false, 0.1f);
    }

    // 평타 어빌리티 실행
    AbilitySystemComponent->TryActivateAbilityByClass(BasicAttackAbilityClass);
}

AActor* APlayerRanger::FindNearestTarget()
{
    float MinDistance = AutoTargetRange;
    AActor* NearestTarget = nullptr;

    // 범위 내 액터 찾기
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(AutoTargetRange),
        QueryParams
    );

    DrawDebugSphere(GetWorld(), GetActorLocation(), AutoTargetRange, 10, FColor::White, false, 0.1f);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* PotentialTarget = Overlap.GetActor();
        
        // GAS를 가진 액터만 타겟으로
        if (PotentialTarget && PotentialTarget->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
        {
            float Distance = FVector::Dist(GetActorLocation(), PotentialTarget->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestTarget = PotentialTarget;
            }
        }
    }

    return NearestTarget;
}

FVector APlayerRanger::GetAttackDirection()
{
    // 타겟이 있으면 타겟 방향
    if (CurrentTarget)
    {
        FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f;
        return Direction.GetSafeNormal();
    }
    
    // 타겟이 없으면 캐릭터가 바라보는 방향
    return GetActorForwardVector();
}

void APlayerRanger::SetJoystickInput(FVector2D JoystickValue)
{
    MovementInput = JoystickValue;
    
    if (Controller != nullptr && MovementInput.Size() > 0.1f)
    {
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementInput.Y);
        AddMovementInput(RightDirection, MovementInput.X);
    }
}

void APlayerRanger::SetAimInput(FVector2D AimValue)
{
    AimInput = AimValue;
    
    if (AimValue.Size() > 0.1f)
    {
        bIsAiming = true;
    }
    else
    {
        bIsAiming = false;
    }
}

AActor* APlayerRanger::GetCurrentTarget() const
{
    return CurrentTarget;
}
