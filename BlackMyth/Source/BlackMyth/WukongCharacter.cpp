// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
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
        
        // 使用 Paragon 的动画蓝图（需要先 Reparent 到 WukongAnimInstance）
        // 参见 Docs/ParagonAnimBP_Integration_Guide.md
        static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
            TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/SunWukong_AnimBlueprint")
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
        AttackMontages.Add(Attack1Asset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack2Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_B_Slow_Montage")
    );
    if (Attack2Asset.Succeeded())
    {
        AttackMontage2 = Attack2Asset.Object;
        AttackMontages.Add(Attack2Asset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack3Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_C_Slow_Montage")
    );
    if (Attack3Asset.Succeeded())
    {
        AttackMontage3 = Attack3Asset.Object;
        AttackMontages.Add(Attack3Asset.Object);
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

    // Also try to auto-load common dodge montages (if present) to ensure montage playback
    static ConstructorHelpers::FObjectFinder<UAnimMontage> DodgeMontage1Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/RMB_Evade_CC_Montage")
    );
    if (DodgeMontage1Asset.Succeeded())
    {
        DodgeMontage = DodgeMontage1Asset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> DodgeMontage2Asset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/AM_Dodge")
    );
    if (DodgeMontage2Asset.Succeeded())
    {
        DodgeMontage = DodgeMontage2Asset.Object;
    }

    // Auto-load hit react animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> HitFrontAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/HitReact_Front")
    );
    if (HitFrontAsset.Succeeded())
    {
        HitReactFrontAnimation = HitFrontAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> HitBackAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/HitReact_Back")
    );
    if (HitBackAsset.Succeeded())
    {
        HitReactBackAnimation = HitBackAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> HitLeftAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/HitReact_Left")
    );
    if (HitLeftAsset.Succeeded())
    {
        HitReactLeftAnimation = HitLeftAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> HitRightAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/HitReact_Right")
    );
    if (HitRightAsset.Succeeded())
    {
        HitReactRightAnimation = HitRightAsset.Object;
    }

    // Auto-load death animation
    static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Death")
    );
    if (DeathAsset.Succeeded())
    {
        DeathAnimation = DeathAsset.Object;
    }

    // Auto-load knockback animation
    static ConstructorHelpers::FObjectFinder<UAnimSequence> KnockbackAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Knockback")
    );
    if (KnockbackAsset.Succeeded())
    {
        KnockbackAnimation = KnockbackAsset.Object;
    }

    // Auto-load stun animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> StunStartAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Stun_Start")
    );
    if (StunStartAsset.Succeeded())
    {
        StunStartAnimation = StunStartAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> StunLoopAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Stun_Loop")
    );
    if (StunLoopAsset.Succeeded())
    {
        StunLoopAnimation = StunLoopAsset.Object;
    }

    // Auto-load emote animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> TauntAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Emote_MonkeyTaunt")
    );
    if (TauntAsset.Succeeded())
    {
        EmoteTauntAnimation = TauntAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> StaffSpinAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Emote_StaffSpin")
    );
    if (StaffSpinAsset.Succeeded())
    {
        EmoteStaffSpinAnimation = StaffSpinAsset.Object;
    }

    // Auto-load ability animations
    static ConstructorHelpers::FObjectFinder<UAnimSequence> FlipFwdAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Fwd")
    );
    if (FlipFwdAsset.Succeeded())
    {
        AbilityFlipForwardAnimation = FlipFwdAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> SlamAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Slam")
    );
    if (SlamAsset.Succeeded())
    {
        AbilitySlamAnimation = SlamAsset.Object;
    }

    // Auto-load air attack animation
    static ConstructorHelpers::FObjectFinder<UAnimSequence> AirAttackAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_Air")
    );
    if (AirAttackAsset.Succeeded())
    {
        AirAttackAnimation = AirAttackAsset.Object;
    }

    // ========== Auto-load Input Actions ==========
    // 加载输入动作资产，确保输入绑定能正常工作
    static ConstructorHelpers::FObjectFinder<UInputAction> DodgeActionAsset(
        TEXT("/Game/_BlackMythGame/Input/Actions/IA_Dodge")
    );
    if (DodgeActionAsset.Succeeded())
    {
        DodgeAction = DodgeActionAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> AttackActionAsset(
        TEXT("/Game/_BlackMythGame/Input/Actions/IA_Attack")
    );
    if (AttackActionAsset.Succeeded())
    {
        AttackAction = AttackActionAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionAsset(
        TEXT("/Game/_BlackMythGame/Input/Actions/IA_Sprint")
    );
    if (SprintActionAsset.Succeeded())
    {
        SprintAction = SprintActionAsset.Object;
    }

    // 加载 Q 键战技输入动作
    static ConstructorHelpers::FObjectFinder<UInputAction> AbilityActionAsset(
        TEXT("/Game/_BlackMythGame/Input/Actions/IA_Ability")
    );
    if (AbilityActionAsset.Succeeded())
    {
        AbilityAction = AbilityActionAsset.Object;
    }

    // 加载地面战技蒙太奇（Q_Flip_Bwd - 后空翻）
    static ConstructorHelpers::FObjectFinder<UAnimMontage> AbilityMontageAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Bwd_Montage")
    );
    if (AbilityMontageAsset.Succeeded())
    {
        AbilityMontage = AbilityMontageAsset.Object;
    }

    // 加载空中战技蒙太奇（Q_Fall_Loop - 下坠攻击）
    static ConstructorHelpers::FObjectFinder<UAnimMontage> AirAbilityMontageAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Fall_Loop_Montage")
    );
    if (AirAbilityMontageAsset.Succeeded())
    {
        AirAbilityMontage = AirAbilityMontageAsset.Object;
    }

    // 加载空中攻击蒙太奇（Primary_Melee_Air）
    static ConstructorHelpers::FObjectFinder<UAnimMontage> AirAttackMontageAsset(
        TEXT("/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_Air_Montage")
    );
    if (AirAttackMontageAsset.Succeeded())
    {
        AirAttackMontage = AirAttackMontageAsset.Object;
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

    // Configure movement speeds and physics
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = WalkSpeed;
        CachedMaxWalkSpeed = WalkSpeed;  // 初始化缓存速度
        
        // 增加重力，让跳跃更有重量感
        Movement->GravityScale = GravityScale;
        
        // 空中控制设为 0，跳跃后不能改变方向（只保留初速度）
        Movement->AirControl = AirControl;
        
        // 关键：空中减速设为 0，这样水平速度在空中不会衰减
        // 默认值约 200-1500，会让角色在空中快速失去水平速度
        Movement->BrakingDecelerationFalling = BrakingDecelerationFalling;
        
        // 跳跃初速度
        Movement->JumpZVelocity = JumpVelocity;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BeginPlay: WalkSpeed=%f, GravityScale=%f, AirControl=%f, BrakingDecelFalling=%f, JumpVelocity=%f"), 
        WalkSpeed, GravityScale, AirControl, BrakingDecelerationFalling, JumpVelocity);

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

    // Update attack cooldown timer
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

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

    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent called!"));
    UE_LOG(LogTemp, Warning, TEXT("  DodgeAction=%s"), DodgeAction ? *DodgeAction->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  AttackAction=%s"), AttackAction ? *AttackAction->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  SprintAction=%s"), SprintAction ? *SprintAction->GetName() : TEXT("NULL"));

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("  EnhancedInputComponent is valid"));
        
        // Bind dodge action
        if (DodgeAction)
        {
            EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AWukongCharacter::OnDodgePressed);
            UE_LOG(LogTemp, Warning, TEXT("  Bound DodgeAction to OnDodgePressed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  DodgeAction is NULL! Dodge will not work!"));
        }

        // Bind attack action
        if (AttackAction)
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWukongCharacter::Attack);
            UE_LOG(LogTemp, Warning, TEXT("  Bound AttackAction to Attack"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  AttackAction is NULL! Attack will not work!"));
        }

        // Bind sprint action
        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AWukongCharacter::OnSprintStarted);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AWukongCharacter::OnSprintStopped);
            UE_LOG(LogTemp, Warning, TEXT("  Bound SprintAction"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  SprintAction is NULL! Sprint will not work!"));
        }

        // Bind ability action (Q key)
        if (AbilityAction)
        {
            EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Started, this, &AWukongCharacter::OnAbilityPressed);
            UE_LOG(LogTemp, Warning, TEXT("  Bound AbilityAction (Q) to OnAbilityPressed"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  AbilityAction is NULL - Q ability disabled (create IA_Ability asset)"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("  EnhancedInputComponent is NULL! No inputs will work!"));
    }
}

void AWukongCharacter::Attack()
{
    // 检查状态 - 翻滚、硬直、死亡时不能攻击
    if (CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead)
    {
        return;
    }

    // 检查攻击冷却 - 防止点击过快
    if (AttackCooldownTimer > 0.0f)
    {
        return;
    }

    // 检查是否有攻击动画
    if (AttackMontages.Num() == 0)
    {
        return;
    }

    // 如果已经在攻击中，加入输入缓冲（用于Combo）
    if (CurrentState == EWukongState::Attacking)
    {
        InputBuffer.Add(TEXT("Attack"));
        return;
    }

    // 执行攻击
    PerformAttack();
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
    UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() called! CurrentState=%d"), (int32)CurrentState);
    
    if (CurrentState == EWukongState::Attacking || 
        CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() blocked by state"));
        return;
    }

    // 空中不能翻滚
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        if (Movement->IsFalling())
        {
            UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() blocked - cannot dodge in air"));
            return;
        }
    }

    if (!IsCooldownActive(TEXT("Dodge")))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() -> calling PerformDodge()"));
        PerformDodge();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() blocked by cooldown"));
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

    // 退出当前状态时的清理逻辑
    if (CurrentState == EWukongState::Attacking)
    {
        // 攻击状态结束，恢复移动能力
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = CachedMaxWalkSpeed > 0.0f ? CachedMaxWalkSpeed : WalkSpeed;
            UE_LOG(LogTemp, Log, TEXT("ChangeState: Exiting Attacking, restored MaxWalkSpeed=%f"), Movement->MaxWalkSpeed);
        }
    }

    PreviousState = CurrentState;
    CurrentState = NewState;

    // State entry logic
    switch (CurrentState)
    {
    case EWukongState::Idle:
    case EWukongState::Moving:
        // 确保进入正常移动状态时速度是正确的
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            if (Movement->MaxWalkSpeed < 1.0f)  // 如果速度异常低
            {
                Movement->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
                UE_LOG(LogTemp, Warning, TEXT("ChangeState: Fixed abnormal MaxWalkSpeed, now=%f"), Movement->MaxWalkSpeed);
            }
        }
        break;
    case EWukongState::Attacking:
        AttackTimer = AttackDuration;
        // 攻击时：允许移动但速度减半
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            // 保存当前最大速度，攻击结束后恢复
            CachedMaxWalkSpeed = Movement->MaxWalkSpeed;
            // 攻击时移动速度变为原来的 50%（可通过 AttackMoveSpeedMultiplier 调整）
            Movement->MaxWalkSpeed = CachedMaxWalkSpeed * AttackMoveSpeedMultiplier;
            UE_LOG(LogTemp, Log, TEXT("ChangeState: Entering Attacking, MaxWalkSpeed reduced to %f (%.0f%%)"), 
                Movement->MaxWalkSpeed, AttackMoveSpeedMultiplier * 100.0f);
        }
        break;
    case EWukongState::Dodging:
        DodgeTimer = DodgeDuration;
        bIsInvincible = true;
        InvincibilityTimer = DodgeInvincibilityDuration;
        StartCooldown(TEXT("Dodge"), DodgeCooldown);
        break;
    case EWukongState::UsingAbility:
        bIsUsingAbility = true;
        AbilityTimer = 1.5f;  // 战技持续时间
        bIsInvincible = true;  // 战技期间无敌
        InvincibilityTimer = 1.5f;
        StartCooldown(TEXT("Ability"), AbilityCooldown);
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
    case EWukongState::UsingAbility:
        UpdateAbilityState(DeltaTime);
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
        // 攻击结束，恢复移动能力
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            // 恢复原来的最大速度
            Movement->MaxWalkSpeed = CachedMaxWalkSpeed > 0.0f ? CachedMaxWalkSpeed : WalkSpeed;
        }

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
    
    // 设置攻击冷却，防止点击过快
    AttackCooldownTimer = AttackCooldown;

    // 检查是否在空中
    bool bIsInAir = false;
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        bIsInAir = Movement->IsFalling();
    }

    // Play attack animation montage
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (bIsInAir)
        {
            // ========== 空中攻击：Primary_Melee_Air ==========
            UE_LOG(LogTemp, Log, TEXT("PerformAttack: In Air - using AirAttackMontage"));
            
            if (AirAttackMontage)
            {
                float Duration = AnimInstance->Montage_Play(AirAttackMontage, 1.0f);
                UE_LOG(LogTemp, Log, TEXT("PerformAttack: Playing AirAttackMontage, Duration=%f"), Duration);
            }
            else if (AirAttackAnimation)
            {
                // 回退：使用动画序列动态创建蒙太奇
                float Duration = PlayAnimationAsMontageDynamic(AirAttackAnimation, FName("DefaultSlot"), 1.0f);
                UE_LOG(LogTemp, Log, TEXT("PerformAttack: Playing AirAttackAnimation as dynamic montage, Duration=%f"), Duration);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformAttack: No air attack animation! Create Primary_Melee_Air_Montage in editor."));
            }
            
            // 空中攻击不增加连击计数
        }
        else
        {
            // ========== 地面攻击：Combo 连击 ==========
            CurrentComboIndex++;
            if (CurrentComboIndex > MaxComboCount)
            {
                CurrentComboIndex = 1;
            }

            LastAttackTime = GetWorld()->GetTimeSeconds();
            
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
                UE_LOG(LogTemp, Log, TEXT("PerformAttack: Ground combo %d"), CurrentComboIndex);
            }
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
    UE_LOG(LogTemp, Log, TEXT("PerformDodge() called"));
    
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
            // Log the montage's slot name for debugging
            if (DodgeMontage->SlotAnimTracks.Num() > 0)
            {
                FName SlotName = DodgeMontage->SlotAnimTracks[0].SlotName;
                UE_LOG(LogTemp, Warning, TEXT("PerformDodge: DodgeMontage=%s, uses Slot='%s'"), 
                    *DodgeMontage->GetName(), *SlotName.ToString());
            }
            
            float Duration = AnimInstance->Montage_Play(DodgeMontage, 1.0f);
            UE_LOG(LogTemp, Warning, TEXT("PerformDodge: Montage_Play returned Duration=%f"), Duration);
            
            if (Duration <= 0.0f)
            {
                UE_LOG(LogTemp, Error, TEXT("PerformDodge: Montage failed to play! Check if AnimBP has matching Slot node!"));
            }
        }
        // If no montage, try to play sequence directly (less flexible but works)
        else if (DodgeAnimation)
        {
            UE_LOG(LogTemp, Log, TEXT("PerformDodge: Trying dynamic montage from DodgeAnimation"));
            // Try to play the dodge animation using several common slot names
            // to improve compatibility with Paragon AnimBP slot naming.
            float Played = PlayAnimationAsMontageDynamic(DodgeAnimation, FName("DefaultGroup.FullBody"), 1.0f);
            if (Played <= 0.0f)
            {
                // Fallbacks
                Played = PlayAnimationAsMontageDynamic(DodgeAnimation, FName("FullBody"), 1.0f);
            }
            if (Played <= 0.0f)
            {
                Played = PlayAnimationAsMontageDynamic(DodgeAnimation, FName("DefaultSlot"), 1.0f);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PerformDodge: No DodgeMontage or DodgeAnimation available!"));
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

float AWukongCharacter::CalculateDamage(bool bIsHeavyAttack, bool bIsAirAttack, int32 ComboIndex) const
{
    // 基础伤害 = 基础攻击力
    float FinalDamage = BaseAttackPower;
    
    // 应用攻击类型倍率
    if (bIsAirAttack)
    {
        // 空中攻击倍率
        FinalDamage *= AirAttackMultiplier;
    }
    else if (bIsHeavyAttack)
    {
        // 重击倍率
        FinalDamage *= HeavyAttackMultiplier;
    }
    else
    {
        // 轻击倍率
        FinalDamage *= LightAttackMultiplier;
    }
    
    // 应用连击倍率（如果有效）
    if (ComboMultipliers.IsValidIndex(ComboIndex))
    {
        FinalDamage *= ComboMultipliers[ComboIndex];
    }
    
    return FinalDamage;
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

float AWukongCharacter::PlayAnimationAsMontageDynamic(UAnimSequence* AnimSequence, FName SlotName, float PlayRate)
{
    if (!AnimSequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayAnimationAsMontageDynamic: AnimSequence is null"));
        return 0.0f;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return 0.0f;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        return 0.0f;
    }

    // Try to create and play montage with the requested slot name first
    TArray<FName> SlotCandidates;
    if (!SlotName.IsNone())
    {
        SlotCandidates.Add(SlotName);
    }
    // Common Paragon/UE slot fallbacks
    SlotCandidates.Add(FName("DefaultGroup.FullBody"));
    SlotCandidates.Add(FName("FullBody"));
    // Paragon AnimBP may use UpperBody slot for many actions
    SlotCandidates.Add(FName("DefaultGroup.UpperBody"));
    SlotCandidates.Add(FName("UpperBody"));
    SlotCandidates.Add(FName("DefaultSlot"));
    SlotCandidates.Add(FName("Default"));

    for (const FName& CandidateSlot : SlotCandidates)
    {
        UAnimMontage* TempMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(
            AnimSequence,
            CandidateSlot,
            0.12f, // BlendInTime (snappier)
            0.12f, // BlendOutTime
            PlayRate
        );

        if (!TempMontage)
        {
            continue;
        }

        const float PlayedDuration = AnimInstance->Montage_Play(TempMontage, PlayRate);
        const float MontageLength = TempMontage->GetPlayLength();
        const float Duration = (PlayedDuration > 0.0f) ? PlayedDuration : MontageLength / FMath::Max(PlayRate, KINDA_SMALL_NUMBER);

        if (Duration > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("Playing dynamic montage: %s in slot %s, duration: %.2f"),
                *AnimSequence->GetName(), *CandidateSlot.ToString(), Duration);
            return Duration;
        }
    }

    // 如果所有候选 Slot 都失败，记录警告
    UE_LOG(LogTemp, Warning, TEXT("PlayAnimationAsMontageDynamic: Failed to create/play montage for %s (tried slots)."), *AnimSequence->GetName());
    return 0.0f;
}

// ========== 战技系统实现 ==========

void AWukongCharacter::OnAbilityPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("OnAbilityPressed() called! CurrentState=%d"), (int32)CurrentState);

    // 检查状态 - 翻滚、硬直、死亡、使用技能时不能释放战技
    if (CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::UsingAbility)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnAbilityPressed() blocked by state"));
        return;
    }

    // 检查战技冷却
    if (IsCooldownActive(TEXT("Ability")))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnAbilityPressed() blocked by cooldown"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnAbilityPressed() -> calling PerformAbility()"));
    PerformAbility();
}

void AWukongCharacter::PerformAbility()
{
    UE_LOG(LogTemp, Log, TEXT("PerformAbility() called"));

    // 切换到使用战技状态
    ChangeState(EWukongState::UsingAbility);

    // 检查是否在空中
    bool bIsInAir = false;
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        bIsInAir = Movement->IsFalling();
    }

    // 播放战技动画
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (bIsInAir)
        {
            // ========== 空中战技：Q_Fall_Loop ==========
            UE_LOG(LogTemp, Log, TEXT("PerformAbility: In Air - using AirAbilityMontage"));
            
            if (AirAbilityMontage)
            {
                if (AirAbilityMontage->SlotAnimTracks.Num() > 0)
                {
                    FName SlotName = AirAbilityMontage->SlotAnimTracks[0].SlotName;
                    UE_LOG(LogTemp, Warning, TEXT("PerformAbility: AirAbilityMontage=%s, uses Slot='%s'"), 
                        *AirAbilityMontage->GetName(), *SlotName.ToString());
                }
                
                float Duration = AnimInstance->Montage_Play(AirAbilityMontage, 1.0f);
                UE_LOG(LogTemp, Warning, TEXT("PerformAbility: Montage_Play returned Duration=%f"), Duration);
                AbilityTimer = FMath::Max(Duration, 1.0f);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformAbility: No AirAbilityMontage! Create Q_Fall_Loop_Montage in editor."));
                AbilityTimer = 0.5f;
            }
            
            // 空中战技：快速下坠
            if (UCharacterMovementComponent* Movement = GetCharacterMovement())
            {
                FVector DownVelocity = FVector(0, 0, -800.0f);  // 快速下坠
                LaunchCharacter(DownVelocity, false, true);
            }
        }
        else
        {
            // ========== 地面战技：Q_Flip_Bwd（后空翻）==========
            UE_LOG(LogTemp, Log, TEXT("PerformAbility: On Ground - using AbilityMontage"));
            
            if (AbilityMontage)
            {
                if (AbilityMontage->SlotAnimTracks.Num() > 0)
                {
                    FName SlotName = AbilityMontage->SlotAnimTracks[0].SlotName;
                    UE_LOG(LogTemp, Warning, TEXT("PerformAbility: AbilityMontage=%s, uses Slot='%s'"), 
                        *AbilityMontage->GetName(), *SlotName.ToString());
                }
                
                float Duration = AnimInstance->Montage_Play(AbilityMontage, 1.0f);
                UE_LOG(LogTemp, Warning, TEXT("PerformAbility: Montage_Play returned Duration=%f"), Duration);
                AbilityTimer = FMath::Max(Duration, 1.0f);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformAbility: No AbilityMontage! Create Q_Flip_Bwd_Montage in editor."));
                AbilityTimer = 0.5f;
            }
            
            // 地面战技：向后小跳
            if (UCharacterMovementComponent* Movement = GetCharacterMovement())
            {
                FVector BackDirection = -GetActorForwardVector();
                FVector LaunchVelocity = BackDirection * 200.0f + FVector(0, 0, 400.0f);
                LaunchCharacter(LaunchVelocity, false, true);
            }
        }
    }
}

void AWukongCharacter::UpdateAbilityState(float DeltaTime)
{
    AbilityTimer -= DeltaTime;

    // 战技结束
    if (AbilityTimer <= 0.0f)
    {
        bIsUsingAbility = false;
        bIsInvincible = false;
        
        // 在战技结束时造成 AOE 伤害（可选）
        // TODO: 实现 AOE 伤害检测
        
        // 根据是否有移动输入决定切换到什么状态
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
