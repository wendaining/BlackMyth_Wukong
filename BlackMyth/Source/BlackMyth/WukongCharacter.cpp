// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"

// Sets default values
AWukongCharacter::AWukongCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Initialize health
    CurrentHealth = MaxHealth;

    // Auto-load Paragon Wukong skeletal mesh
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> WukongMeshAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Meshes/Wukong")
    );
    if (WukongMeshAsset.Succeeded() && GetMesh())
    {
        GetMesh()->SetSkeletalMesh(WukongMeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        
        // Try to load custom AnimBP based on WukongAnimInstance
        static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
            TEXT("/Game/Blueprints/ABP_Wukong")
        );
        if (AnimBPClass.Succeeded())
        {
            GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
        }
    }

    // Auto-load attack montages
    static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack1Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_A_Slow_Montage")
    );
    if (Attack1Asset.Succeeded())
    {
        AttackMontage1 = Attack1Asset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack2Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_B_Slow_Montage")
    );
    if (Attack2Asset.Succeeded())
    {
        AttackMontage2 = Attack2Asset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack3Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_C_Slow_Montage")
    );
    if (Attack3Asset.Succeeded())
    {
        AttackMontage3 = Attack3Asset.Object;
    }

    // Auto-load locomotion animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleAnimAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Idle")
    );
    if (IdleAnimAsset.Succeeded())
    {
        IdleAnimation = IdleAnimAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkAnimAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Jog_Fwd")
    );
    if (WalkAnimAsset.Succeeded())
    {
        WalkForwardAnimation = WalkAnimAsset.Object;
    }

    // Note: Sprint uses the same Jog animation, speed will be controlled by AnimBP
    SprintForwardAnimation = WalkForwardAnimation;

    // Auto-load jump animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpStartAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Jump_Start")
    );
    if (JumpStartAsset.Succeeded())
    {
        JumpStartAnimation = JumpStartAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpApexAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Jump_Apex")
    );
    if (JumpApexAsset.Succeeded())
    {
        JumpApexAnimation = JumpApexAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpLandAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Jump_Land")
    );
    if (JumpLandAsset.Succeeded())
    {
        JumpLandAnimation = JumpLandAsset.Object;
    }

    // Auto-load dodge animation
    static ConstructorHelpers::FObjectFinder<UAnimSequence> DodgeAnimAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/RMB_Evade_CC")
    );
    if (DodgeAnimAsset.Succeeded())
    {
        DodgeAnimation = DodgeAnimAsset.Object;
    }

    // Create Combat Component (will be implemented by Member C)
    // CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

// Called when the game starts or when spawned
void AWukongCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    CurrentState = EWukongState::Idle;

    // Configure movement speeds
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = WalkSpeed;
    }

    // Broadcast initial health
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

// Called every frame
void AWukongCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update state machine
    UpdateState(DeltaTime);

    // Update cooldowns
    UpdateCooldowns(DeltaTime);

    // Update invincibility timer
    if (bIsInvincible && InvincibilityTimer > 0.0f)
    {
        InvincibilityTimer -= DeltaTime;
        if (InvincibilityTimer <= 0.0f)
        {
            bIsInvincible = false;
        }
    }
}

// Called to bind functionality to input
void AWukongCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind dodge action
        if (DodgeAction)
        {
            EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AWukongCharacter::OnDodgePressed);
        }

        // Bind attack action
        if (AttackAction)
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWukongCharacter::Attack);
        }

        // Bind sprint action
        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AWukongCharacter::OnSprintStarted);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AWukongCharacter::OnSprintStopped);
        }
    }
}

void AWukongCharacter::Attack()
{
    if (AttackMontages.Num() == 0)
    {
        return;
    }

    const int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
    if (UAnimMontage* SelectedMontage = AttackMontages[RandomIndex])
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInstance->Montage_Play(SelectedMontage, 1.0f);
        }
    }
}

// Public Interface Implementation
void AWukongCharacter::ReceiveDamage(float Damage, AActor* DamageInstigator)
{
    if (CurrentState == EWukongState::Dead || bIsInvincible)
    {
        return;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        ChangeState(EWukongState::HitStun);
        HitStunTimer = HitStunDuration;
    }
}

void AWukongCharacter::SetInvincible(bool bInInvincible)
{
    bIsInvincible = bInInvincible;
    if (!bIsInvincible)
    {
        InvincibilityTimer = 0.0f;
    }
}

// Input Handlers
void AWukongCharacter::OnDodgePressed()
{
    if (CurrentState == EWukongState::Attacking || 
        CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead)
    {
        return;
    }

    if (!IsCooldownActive(TEXT("Dodge")))
    {
        PerformDodge();
    }
}

void AWukongCharacter::OnAttackPressed()
{
    if (CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead)
    {
        return;
    }

    // Add to input buffer
    InputBuffer.Add(TEXT("Attack"));

    // Process input immediately if not attacking
    if (CurrentState != EWukongState::Attacking)
    {
        ProcessInputBuffer();
    }
}

void AWukongCharacter::OnSprintStarted()
{
    if (CurrentState == EWukongState::Attacking || 
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead)
    {
        return;
    }

    bIsSprinting = true;
    UpdateMovementSpeed();
}

void AWukongCharacter::OnSprintStopped()
{
    bIsSprinting = false;
    UpdateMovementSpeed();
}

// State Management
void AWukongCharacter::ChangeState(EWukongState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    PreviousState = CurrentState;
    CurrentState = NewState;

    // State entry logic
    switch (CurrentState)
    {
    case EWukongState::Attacking:
        AttackTimer = AttackDuration;
        break;
    case EWukongState::Dodging:
        DodgeTimer = DodgeDuration;
        bIsInvincible = true;
        InvincibilityTimer = DodgeInvincibilityDuration;
        StartCooldown(TEXT("Dodge"), DodgeCooldown);
        break;
    case EWukongState::HitStun:
        ResetCombo();
        InputBuffer.Empty();
        break;
    case EWukongState::Dead:
        ResetCombo();
        InputBuffer.Empty();
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->DisableMovement();
        }
        break;
    default:
        break;
    }
}

void AWukongCharacter::UpdateState(float DeltaTime)
{
    switch (CurrentState)
    {
    case EWukongState::Idle:
        UpdateIdleState(DeltaTime);
        break;
    case EWukongState::Moving:
        UpdateMovingState(DeltaTime);
        break;
    case EWukongState::Attacking:
        UpdateAttackingState(DeltaTime);
        break;
    case EWukongState::Dodging:
        UpdateDodgingState(DeltaTime);
        break;
    case EWukongState::HitStun:
        UpdateHitStunState(DeltaTime);
        break;
    case EWukongState::Dead:
        UpdateDeadState(DeltaTime);
        break;
    }
}

void AWukongCharacter::UpdateIdleState(float DeltaTime)
{
    FVector InputDirection = GetMovementInputDirection();
    if (!InputDirection.IsNearlyZero())
    {
        ChangeState(EWukongState::Moving);
    }
}

void AWukongCharacter::UpdateMovingState(float DeltaTime)
{
    FVector InputDirection = GetMovementInputDirection();
    if (InputDirection.IsNearlyZero())
    {
        ChangeState(EWukongState::Idle);
    }
}

void AWukongCharacter::UpdateAttackingState(float DeltaTime)
{
    AttackTimer -= DeltaTime;

    // Process input buffer during attack window
    if (AttackTimer < AttackDuration * 0.5f && InputBuffer.Num() > 0)
    {
        ProcessInputBuffer();
    }

    if (AttackTimer <= 0.0f)
    {
        // Check for combo timeout
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        if (TimeSinceLastAttack > ComboResetTime)
        {
            ResetCombo();
        }

        // Return to appropriate state
        FVector InputDirection = GetMovementInputDirection();
        if (InputDirection.IsNearlyZero())
        {
            ChangeState(EWukongState::Idle);
        }
        else
        {
            ChangeState(EWukongState::Moving);
        }
    }
}

void AWukongCharacter::UpdateDodgingState(float DeltaTime)
{
    DodgeTimer -= DeltaTime;
    UpdateDodgeMovement(DeltaTime);

    if (DodgeTimer <= 0.0f)
    {
        bIsDodging = false;
        FVector InputDirection = GetMovementInputDirection();
        if (InputDirection.IsNearlyZero())
        {
            ChangeState(EWukongState::Idle);
        }
        else
        {
            ChangeState(EWukongState::Moving);
        }
    }
}

void AWukongCharacter::UpdateHitStunState(float DeltaTime)
{
    HitStunTimer -= DeltaTime;

    if (HitStunTimer <= 0.0f)
    {
        FVector InputDirection = GetMovementInputDirection();
        if (InputDirection.IsNearlyZero())
        {
            ChangeState(EWukongState::Idle);
        }
        else
        {
            ChangeState(EWukongState::Moving);
        }
    }
}

void AWukongCharacter::UpdateDeadState(float DeltaTime)
{
    // Dead state - no updates needed
}

// Combat Methods
void AWukongCharacter::PerformAttack()
{
    ChangeState(EWukongState::Attacking);
    
    CurrentComboIndex++;
    if (CurrentComboIndex > MaxComboCount)
    {
        CurrentComboIndex = 1;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Play attack animation montage
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        UAnimMontage* MontageToPlay = nullptr;
        
        switch (CurrentComboIndex)
        {
        case 1: MontageToPlay = AttackMontage1; break;
        case 2: MontageToPlay = AttackMontage2; break;
        case 3: MontageToPlay = AttackMontage3; break;
        }
        
        if (MontageToPlay)
        {
            AnimInstance->Montage_Play(MontageToPlay, 1.0f);
        }
    }

    // TODO: Deal damage via CombatComponent
    // if (CombatComponent)
    // {
    //     CombatComponent->ExecuteAttack(CurrentComboIndex);
    // }
}

void AWukongCharacter::ResetCombo()
{
    CurrentComboIndex = 0;
    LastAttackTime = 0.0f;
}

void AWukongCharacter::ProcessInputBuffer()
{
    if (InputBuffer.Num() == 0)
    {
        return;
    }

    // Remove old inputs from buffer
    float CurrentTime = GetWorld()->GetTimeSeconds();
    InputBuffer.RemoveAll([CurrentTime, this](const FString& Input) {
        return (CurrentTime - LastAttackTime) > InputBufferTime;
    });

    // Process first input in buffer
    if (InputBuffer.Num() > 0)
    {
        FString NextInput = InputBuffer[0];
        InputBuffer.RemoveAt(0);

        if (NextInput == TEXT("Attack"))
        {
            PerformAttack();
        }
    }
}

// Dodge Methods
void AWukongCharacter::PerformDodge()
{
    ChangeState(EWukongState::Dodging);

    // Determine dodge direction
    FVector InputDirection = GetMovementInputDirection();
    if (InputDirection.IsNearlyZero())
    {
        DodgeDirection = GetActorForwardVector();
    }
    else
    {
        DodgeDirection = InputDirection;
    }

    DodgeDirection.Normalize();
    bIsDodging = true;

    // Play dodge animation
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        // Try to play montage first
        if (DodgeMontage)
        {
            AnimInstance->Montage_Play(DodgeMontage, 1.0f);
        }
        // If no montage, try to play sequence directly (less flexible but works)
        else if (DodgeAnimation)
        {
            // Create a temporary montage from the sequence
            UAnimMontage* TempMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(
                DodgeAnimation, 
                FName("DefaultSlot"), 
                0.25f,  // Blend in time
                0.25f,  // Blend out time
                1.0f    // Play rate
            );
            
            if (TempMontage)
            {
                AnimInstance->Montage_Play(TempMontage, 1.0f);
            }
        }
    }
}

void AWukongCharacter::UpdateDodgeMovement(float DeltaTime)
{
    if (!bIsDodging)
    {
        return;
    }

    // Calculate dodge velocity based on remaining time
    float DodgeSpeed = DodgeDistance / DodgeDuration;
    FVector DodgeVelocity = DodgeDirection * DodgeSpeed * DeltaTime;

    // Apply dodge movement
    FHitResult HitResult;
    AddActorWorldOffset(DodgeVelocity, true, &HitResult);
}

// Cooldown Management
bool AWukongCharacter::IsCooldownActive(const FString& CooldownName) const
{
    const float* CooldownTime = CooldownMap.Find(CooldownName);
    return (CooldownTime != nullptr && *CooldownTime > 0.0f);
}

void AWukongCharacter::StartCooldown(const FString& CooldownName, float Duration)
{
    CooldownMap.Add(CooldownName, Duration);
}

void AWukongCharacter::UpdateCooldowns(float DeltaTime)
{
    TArray<FString> ExpiredCooldowns;

    for (auto& Cooldown : CooldownMap)
    {
        Cooldown.Value -= DeltaTime;
        if (Cooldown.Value <= 0.0f)
        {
            ExpiredCooldowns.Add(Cooldown.Key);
        }
    }

    // Remove expired cooldowns
    for (const FString& CooldownName : ExpiredCooldowns)
    {
        CooldownMap.Remove(CooldownName);
    }
}

// New accessor implementations
float AWukongCharacter::GetMovementSpeed() const
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        return Movement->Velocity.Size();
    }
    return 0.0f;
}

FVector AWukongCharacter::GetMovementDirection() const
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        FVector Velocity = Movement->Velocity;
        Velocity.Z = 0.0f; // Ignore vertical component
        if (!Velocity.IsNearlyZero())
        {
            return Velocity.GetSafeNormal();
        }
    }
    return FVector::ZeroVector;
}

// Helper Methods
void AWukongCharacter::Die()
{
    ChangeState(EWukongState::Dead);
    OnHealthChanged.Broadcast(0.0f, MaxHealth);

    // TODO: Trigger death animation and respawn logic
}

void AWukongCharacter::UpdateMovementSpeed()
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        if (bIsSprinting)
        {
            Movement->MaxWalkSpeed = SprintSpeed;
        }
        else
        {
            Movement->MaxWalkSpeed = WalkSpeed;
        }
    }
}

FVector AWukongCharacter::GetMovementInputDirection() const
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        return Movement->GetLastInputVector();
    }
    return FVector::ZeroVector;
}
