// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongCharacter.h"
#include "Components/StaminaComponent.h"
#include "Components/CombatComponent.h"
#include "Components/HealthComponent.h"
#include "Combat/TraceHitboxComponent.h"
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

    // 创建生命组件
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // 创建体力组件
    StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));

    // 创建战斗组件
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

    // 创建武器 Hitbox 组件（先挂载到 RootComponent，BeginPlay 时再附加到骨骼）
    WeaponTraceHitbox = CreateDefaultSubobject<UTraceHitboxComponent>(TEXT("WeaponTraceHitbox"));

    // 注意：所有动画资产和输入动作现在都应在蓝图子类 (BP_Wukong_New) 中设置
    // 不再在 C++ 构造函数中硬编码加载路径，以便于在编辑器中灵活配置
}

// Called when the game starts or when spawned
void AWukongCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 确保 AttackMontages 数组已填充（兼容旧的单个属性设置方式）
    if (AttackMontages.Num() == 0)
    {
        if (AttackMontage1) AttackMontages.Add(AttackMontage1);
        if (AttackMontage2) AttackMontages.Add(AttackMontage2);
        if (AttackMontage3) AttackMontages.Add(AttackMontage3);
        
        if (AttackMontages.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("BeginPlay: Populated AttackMontages from individual properties. Count=%d"), AttackMontages.Num());
        }
    }

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

        // 启用 Root Motion 时的强制位移处理
        // 这可以防止某些动画播放完后角色被拉回原位
        // 但更根本的解决方法是在动画资源中启用 Root Motion，并在 AnimBP 中设置 Root Motion Mode
    }
    
    UE_LOG(LogTemp, Log, TEXT("BeginPlay: WalkSpeed=%f, GravityScale=%f, AirControl=%f, BrakingDecelFalling=%f, JumpVelocity=%f"), 
        WalkSpeed, GravityScale, AirControl, BrakingDecelerationFalling, JumpVelocity);

    // 配置 TraceHitboxComponent
    if (WeaponTraceHitbox)
    {
        // 设置武器骨骼
        // 模型自带 weapon_r 骨骼，直接使用
        WeaponTraceHitbox->SetStartSocket(FName("weapon_r"));  // 握把位置（起点）
        
        // 模型自带 weapon_tou (头) 骨骼，作为射线检测终点
        WeaponTraceHitbox->SetEndSocket(FName("weapon_tou"));  // 武器前端（终点）

        // 扫描半径（金箍棒粗细约5-10）
        WeaponTraceHitbox->SetTraceRadius(7.0f);

        // 关联 CombatComponent
        if (CombatComponent)
        {
            WeaponTraceHitbox->SetCombatComponent(CombatComponent);
        }

        // 开启调试绘制
        WeaponTraceHitbox->SetDebugDrawEnabled(true);

        UE_LOG(LogTemp, Log, TEXT("[Wukong] WeaponTraceHitbox configured"));
    }

    // 绑定生命组件死亡事件
    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &AWukongCharacter::OnHealthDepleted);
    }

    // 绑定体力耗尽事件
    if (StaminaComponent)
    {
        StaminaComponent->OnStaminaDepleted.AddDynamic(this, &AWukongCharacter::OnStaminaDepleted);
    }
}

// Called every frame
void AWukongCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update state machine
    UpdateState(DeltaTime);

    // Update cooldowns
    UpdateCooldowns(DeltaTime);

    // 体力更新由StaminaComponent自己处理

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
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformAttack);
            UE_LOG(LogTemp, Warning, TEXT("  Bound AttackAction to PerformAttack"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  AttackAction is NULL! Attack will not work!"));
        }

        // Bind heavy attack action
        if (HeavyAttackAction)
        {
            EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformHeavyAttack);
            UE_LOG(LogTemp, Warning, TEXT("  Bound HeavyAttackAction to PerformHeavyAttack"));
        }

        // Bind pole stance action
        if (PoleStanceAction)
        {
            EnhancedInputComponent->BindAction(PoleStanceAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformPoleStance);
            UE_LOG(LogTemp, Warning, TEXT("  Bound PoleStanceAction to PerformPoleStance"));
        }

        // Bind staff spin action
        if (StaffSpinAction)
        {
            EnhancedInputComponent->BindAction(StaffSpinAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformStaffSpin);
            EnhancedInputComponent->BindAction(StaffSpinAction, ETriggerEvent::Completed, this, &AWukongCharacter::PerformStaffSpin); // Handle release if needed
            UE_LOG(LogTemp, Warning, TEXT("  Bound StaffSpinAction to PerformStaffSpin"));
        }

        // Bind item use action
        if (UseItemAction)
        {
            EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &AWukongCharacter::UseItem);
            UE_LOG(LogTemp, Warning, TEXT("  Bound UseItemAction to UseItem"));
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

        // Bind ability action (Q key) - REMOVED in favor of specific skills
        /*
        if (AbilityAction)
        {
            EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Started, this, &AWukongCharacter::OnAbilityPressed);
            UE_LOG(LogTemp, Warning, TEXT("  Bound AbilityAction (Q) to OnAbilityPressed"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  AbilityAction is NULL - Q ability disabled (create IA_Ability asset)"));
        }
        */
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

    // 检查是否有足够体力开始冲刺（至少需要能持续一小段时间）
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->SprintStaminaCost * 0.1f))
    {
        return;
    }

    bIsSprinting = true;
    // 告诉体力组件开始持续消耗
    StaminaComponent->SetContinuousConsumption(true, StaminaComponent->SprintStaminaCost);
    UpdateMovementSpeed();
}

void AWukongCharacter::OnSprintStopped()
{
    bIsSprinting = false;
    // 停止体力持续消耗
    if (StaminaComponent)
    {
        StaminaComponent->SetContinuousConsumption(false, 0.0f);
    }
    UpdateMovementSpeed();
}

void AWukongCharacter::ReceiveDamage(float Damage, AActor* DamageInstigator)
{
    // 如果已经死亡或处于无敌状态，不受到伤害
    if (CurrentState == EWukongState::Dead || bIsInvincible)
    {
        return;
    }

    // 委托给生命组件扣血
    if (HealthComponent)
    {
        HealthComponent->TakeDamage(Damage, DamageInstigator);
        
        if (HealthComponent->IsDead())
        {
            Die();
            return;
        }
    }

    // 计算受击方向并播放对应动画
    UAnimSequence* HitAnim = HitReactFrontAnimation;
    float HitAnimPlayRate = 1.0f; // 默认播放速度
    
    if (DamageInstigator)
    {
        // 计算攻击来源方向（相对于角色的方向）
        FVector HitDir = (DamageInstigator->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FVector Forward = GetActorForwardVector();
        FVector Right = GetActorRightVector();

        float ForwardDot = FVector::DotProduct(Forward, HitDir);
        float RightDot = FVector::DotProduct(Right, HitDir);

        // 判定受击方向
        // 注意：这里是判断攻击来源。如果攻击来自前方，Dot > 0。
        if (ForwardDot >= 0.5f) // 攻击来自前方
        {
            HitAnim = HitReactFrontAnimation;
        }
        else if (ForwardDot <= -0.5f) // 攻击来自后方
        {
            HitAnim = HitReactBackAnimation;
        }
        else // 侧面
        {
            // 侧面受击动画通常比较长，稍微加快一点播放速度
            HitAnimPlayRate = 1.6f; 

            if (RightDot > 0.0f) // 攻击来自右侧
            {
                HitAnim = HitReactRightAnimation;
            }
            else // 攻击来自左侧
            {
                HitAnim = HitReactLeftAnimation;
            }
        }
    }

    // 播放受击动画（作为动态蒙太奇）
    if (HitAnim)
    {
        // 修复：强制启用根运动 (Root Motion)，防止角色受击移动后瞬移回原位
        // 这会让胶囊体跟随动画的位移
        HitAnim->bEnableRootMotion = true;
        HitAnim->bForceRootLock = true; // 确保根骨骼被锁定，位移应用到胶囊体

        UE_LOG(LogTemp, Warning, TEXT("ReceiveDamage: Playing HitAnim '%s' with Rate %.2f"), *HitAnim->GetName(), HitAnimPlayRate);
        // 优先使用 DefaultSlot
        float Duration = PlayAnimationAsMontageDynamic(HitAnim, FName("DefaultSlot"), HitAnimPlayRate);
        HitStunTimer = (Duration > 0.0f) ? Duration : HitStunDuration;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ReceiveDamage: HitAnim is NULL!"));
        HitStunTimer = HitStunDuration;
    }

    // 进入受击硬直状态
    ChangeState(EWukongState::HitStun);
}

void AWukongCharacter::SetInvincible(bool bInInvincible)
{
    this->bIsInvincible = bInInvincible;
    if (HealthComponent)
    {
        HealthComponent->SetInvincible(bInInvincible);
    }
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
        // 允许体力恢复
        if (StaminaComponent)
        {
            StaminaComponent->SetCanRegenerate(true);
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
        // 攻击时禁止体力恢复
        if (StaminaComponent)
        {
            StaminaComponent->SetCanRegenerate(false);
        }
        break;
    case EWukongState::Dodging:
        DodgeTimer = DodgeDuration;
        bIsInvincible = true;
        InvincibilityTimer = DodgeInvincibilityDuration;
        StartCooldown(TEXT("Dodge"), DodgeCooldown);
        // 翻滚时禁止体力恢复
        if (StaminaComponent)
        {
            StaminaComponent->SetCanRegenerate(false);
        }
        break;
    case EWukongState::UsingAbility:
        bIsUsingAbility = true;
        AbilityTimer = 1.5f;  // 战技持续时间
        bIsInvincible = true;  // 战技期间无敌
        InvincibilityTimer = 1.5f;
        StartCooldown(TEXT("Ability"), AbilityCooldown);
        // 使用战技时禁止体力恢复
        if (StaminaComponent)
        {
            StaminaComponent->SetCanRegenerate(false);
        }
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

    // 攻击期间禁止移动输入生效（但允许转向，如果需要完全锁死转向，可以在 Move 函数里加判断）
    // 注意：这里不需要额外代码，因为在 Move() 函数里我们已经判断了 IsAttacking 就不处理 AddMovementInput

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
        float ComboResetTime = CombatComponent ? CombatComponent->ComboResetTime : 1.0f;
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
    // 检查是否有足够体力攻击
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->AttackStaminaCost))
    {
        return;
    }

    // ========== 攻击间隔保护 (基于动画进度) ==========
    // 如果正在攻击，检查当前蒙太奇播放进度
    if (CurrentState == EWukongState::Attacking)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
            {
                // 获取当前播放位置和总长度
                float CurrentPos = AnimInstance->Montage_GetPosition(CurrentMontage);
                // 注意：GetPlayLength() 返回的是原始长度，不包含 RateScale。
                // 但 Montage_GetPosition 也是基于原始时间的，所以直接比对即可。
                float TotalLength = CurrentMontage->GetPlayLength();

                // 设定允许连招的阈值：动作播放了 80% 之后才允许打断
                // 这样配合 1.5倍速播放，既能看清动作，手感又不会太粘滞
                const float ComboWindowThreshold = 0.8f;

                if (TotalLength > 0.0f && (CurrentPos / TotalLength) < ComboWindowThreshold)
                {
                    // 还没播到 80%，缓冲输入
                    if (InputBuffer.Num() == 0) 
                    {
                        InputBuffer.Add(TEXT("Attack"));
                        UE_LOG(LogTemp, Log, TEXT("PerformAttack: Input Buffered (Progress: %.2f%% < 80%%)"), (CurrentPos / TotalLength) * 100.0f);
                    }
                    return;
                }
            }
        }
    }

    // 消耗体力
    StaminaComponent->ConsumeStamina(StaminaComponent->AttackStaminaCost);

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
            if (CombatComponent)
            {
                CombatComponent->StartAttack();
            }
            int32 ComboIndex = CombatComponent ? CombatComponent->GetCurrentComboIndex() : 0;

            LastAttackTime = GetWorld()->GetTimeSeconds();
            
            UAnimMontage* MontageToPlay = nullptr;
            
            // ComboIndex 从0开始，所以 0=第1段, 1=第2段, 2=第3段
            switch (ComboIndex)
            {
            case 0: MontageToPlay = AttackMontage1; break;
            case 1: MontageToPlay = AttackMontage2; break;
            case 2: MontageToPlay = AttackMontage3; break;
            }
            
            if (MontageToPlay)
            {
                AnimInstance->Montage_Play(MontageToPlay, 1.0f);
                UE_LOG(LogTemp, Log, TEXT("PerformAttack: Ground combo %d"), ComboIndex + 1);
            }
        }
    }
}

void AWukongCharacter::ResetCombo()
{
    if (CombatComponent)
    {
        CombatComponent->ResetCombo();
    }
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
    
    // 检查是否有足够体力翻滚
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->DodgeStaminaCost))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformDodge: Not enough stamina! Current=%f, Required=%f"), 
            StaminaComponent ? StaminaComponent->GetCurrentStamina() : 0.0f,
            StaminaComponent ? StaminaComponent->DodgeStaminaCost : 0.0f);
        return;
    }

    // 消耗体力
    StaminaComponent->ConsumeStamina(StaminaComponent->DodgeStaminaCost);

    ChangeState(EWukongState::Dodging);

    // Determine dodge direction relative to actor
    FVector InputDirection = GetMovementInputDirection();
    UAnimMontage* MontageToPlay = DodgeFwdMontage; // Default to forward

    if (!InputDirection.IsNearlyZero())
    {
        DodgeDirection = InputDirection;
        
        // Calculate dot product to determine direction relative to actor forward
        FVector ActorForward = GetActorForwardVector();
        FVector ActorRight = GetActorRightVector();
        
        float ForwardDot = FVector::DotProduct(ActorForward, InputDirection);
        float RightDot = FVector::DotProduct(ActorRight, InputDirection);

        if (ForwardDot > 0.707f) // Forward
        {
            MontageToPlay = DodgeFwdMontage;
        }
        else if (ForwardDot < -0.707f) // Backward
        {
            MontageToPlay = DodgeBwdMontage;
        }
        else if (RightDot > 0.0f) // Right
        {
            MontageToPlay = DodgeRightMontage;
        }
        else // Left
        {
            MontageToPlay = DodgeLeftMontage;
        }
    }
    else
    {
        DodgeDirection = GetActorForwardVector();
        MontageToPlay = DodgeFwdMontage; // No input, dodge forward (or backward?)
    }

    DodgeDirection.Normalize();
    bIsDodging = true;

    // Play dodge animation
    if (MontageToPlay)
    {
        PlayMontage(MontageToPlay);
    }
    else
    {
        // Fallback to old single montage if specific ones aren't set
        /*
        if (DodgeMontage)
        {
             PlayMontage(DodgeMontage);
        }
        else */ 
        if (DodgeAnimation)
        {
             PlayAnimationAsMontageDynamic(DodgeAnimation, FName("DefaultSlot"), 1.0f);
        }
    }
}

void AWukongCharacter::PerformHeavyAttack()
{
    if (HeavyAttackMontage)
    {
        ChangeState(EWukongState::Attacking);
        PlayMontage(HeavyAttackMontage);
    }
}

void AWukongCharacter::PerformStaffSpin()
{
    if (StaffSpinMontage)
    {
        ChangeState(EWukongState::Attacking); 
        PlayMontage(StaffSpinMontage);
    }
}

void AWukongCharacter::PerformPoleStance()
{
    if (PoleStanceMontage)
    {
        ChangeState(EWukongState::Attacking);
        PlayMontage(PoleStanceMontage);
    }
}

void AWukongCharacter::UseItem()
{
    if (DrinkGourdMontage)
    {
        PlayMontage(DrinkGourdMontage);
    }
}

void AWukongCharacter::PlayMontage(UAnimMontage* MontageToPlay, FName SectionName)
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (MontageToPlay)
        {
            float Duration = AnimInstance->Montage_Play(MontageToPlay);
            if (SectionName != NAME_None)
            {
                AnimInstance->Montage_JumpToSection(SectionName, MontageToPlay);
            }
            UE_LOG(LogTemp, Log, TEXT("PlayMontage: Playing %s, Duration=%f"), *MontageToPlay->GetName(), Duration);
        }
    }
}


void AWukongCharacter::UpdateDodgeMovement(float DeltaTime)
{
    if (!bIsDodging)
    {
        return;
    }

    // 如果启用了 Root Motion，就不需要手动计算位移了
    if (GetMesh()->IsPlayingRootMotion())
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

// ========== 体力耗尽回调 ==========
void AWukongCharacter::OnStaminaDepleted()
{
    // 体力耗尽时停止冲刺
    if (bIsSprinting)
    {
        bIsSprinting = false;
        if (StaminaComponent)
        {
            StaminaComponent->SetContinuousConsumption(false, 0.0f);
        }
        UpdateMovementSpeed();
        UE_LOG(LogTemp, Log, TEXT("OnStaminaDepleted: Stamina depleted, stopping sprint"));
    }
}

// ========== 生命值耗尽回调 ==========
void AWukongCharacter::OnHealthDepleted(AActor* Killer)
{
    UE_LOG(LogTemp, Warning, TEXT("OnHealthDepleted: Character died! Killer=%s"), 
        Killer ? *Killer->GetName() : TEXT("None"));
    Die();
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
    if (CombatComponent)
    {
        return CombatComponent->CalculateDamage(bIsHeavyAttack, bIsAirAttack, ComboIndex);
    }
    return 0.0f;
}

int32 AWukongCharacter::GetComboIndex() const
{
    return CombatComponent ? CombatComponent->GetCurrentComboIndex() : 0;
}

float AWukongCharacter::GetBaseAttackPower() const
{
    return CombatComponent ? CombatComponent->GetBaseAttackPower() : 0.0f;
}

// ========== 跳跃体力检查 ==========

bool AWukongCharacter::CanJumpInternal_Implementation() const
{
    // 先检查父类的跳跃条件
    if (!Super::CanJumpInternal_Implementation())
    {
        return false;
    }
    
    // 检查是否有足够体力跳跃
    if (StaminaComponent && !StaminaComponent->HasEnoughStamina(StaminaComponent->JumpStaminaCost))
    {
        return false;
    }
    
    return true;
}

void AWukongCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    
    // 跳跃时消耗体力
    if (StaminaComponent)
    {
        StaminaComponent->ConsumeStamina(StaminaComponent->JumpStaminaCost);
        UE_LOG(LogTemp, Log, TEXT("OnJumped: Consumed %f stamina, remaining=%f"), 
            StaminaComponent->JumpStaminaCost, StaminaComponent->GetCurrentStamina());
    }

    // 播放跳跃蒙太奇
    if (JumpMontage)
    {
        PlayAnimMontage(JumpMontage, 1.0f, FName("Start"));
    }
}

void AWukongCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // 落地时跳转到 Land Section
    if (JumpMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && AnimInstance->Montage_IsPlaying(JumpMontage))
        {
            AnimInstance->Montage_JumpToSection(FName("Land"), JumpMontage);
        }
        else
        {
             // 如果没在播（比如从高处直接掉下来），直接播 Land
             PlayAnimMontage(JumpMontage, 1.0f, FName("Land"));
        }
    }
}

// Helper Methods
void AWukongCharacter::Die()
{
    ChangeState(EWukongState::Dead);
    // 生命组件会广播死亡事件

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
