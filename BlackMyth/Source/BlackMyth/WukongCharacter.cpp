// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongCharacter.h"
#include "WukongClone.h"
#include "EnemyBase.h"
#include "NPCCharacter.h"
#include "ButterflyPawn.h"
#include "UI/PlayerHUDWidget.h"
#include "UI/InteractionPromptWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaminaComponent.h"
#include "Components/CombatComponent.h"
#include "Components/HealthComponent.h"
#include "Components/TargetingComponent.h"
#include "Components/TeamComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneStateComponent.h"
#include "Combat/TraceHitboxComponent.h"
#include "Dialogue/DialogueComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Temple.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

// 设置默认值
AWukongCharacter::AWukongCharacter()
{
    // 每帧都调用 Tick()， 如果不需要可以关闭以提高性能
    PrimaryActorTick.bCanEverTick = true;

    // ========== 角色旋转设置==========
    // 不让角色自动面向移动方向，这样按 S 时角色不会转身
    bUseControllerRotationYaw = false;  // 不使用控制器旋转
    
    // 获取角色移动组件，并设置旋转行为
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        // 关闭"面向移动方向"，以实现按S不是面向后面，而是播放后退动画
        MovementComp->bOrientRotationToMovement = false;
        
        // 改用"面向控制器方向"，让角色朝向摄像机方向
        MovementComp->bUseControllerDesiredRotation = true;
        MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);  // 旋转速度
    }

    // 创建生命组件
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // 创建体力组件
    StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));

    // 创建战斗组件
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

    // 创建武器 Hitbox 组件（先挂载到 RootComponent，BeginPlay 时再附加到骨骼）
    WeaponTraceHitbox = CreateDefaultSubobject<UTraceHitboxComponent>(TEXT("WeaponTraceHitbox"));

    // 创建目标锁定组件
    TargetingComponent = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));

    // 创建阵营组件（默认为玩家阵营）
    TeamComponent = CreateDefaultSubobject<UTeamComponent>(TEXT("TeamComponent"));

    // 所有动画资产和输入动作都应在蓝图类 (BP_Wukong) 中设置
    // 不在 C++ 构造函数中硬编码加载路径，以便于在编辑器中灵活配置
}

// 当游戏启动或者角色重生时调用
void AWukongCharacter::BeginPlay()
{
    Super::BeginPlay();

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

        // 在动画资源中启用 Root Motion，并在 AnimBP 中设置 Root Motion Mode
        // 以防止完成有位移的动作之后，角色被强制拉回原地
    }
    
    UE_LOG(LogTemp, Log, TEXT("BeginPlay: WalkSpeed=%f, GravityScale=%f, AirControl=%f, BrakingDecelFalling=%f, JumpVelocity=%f"), 
        WalkSpeed, GravityScale, AirControl, BrakingDecelerationFalling, JumpVelocity);

    // 

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

    // 绑定伤害造成事件（用于更新连击计数）
    if (CombatComponent)
    {
        CombatComponent->OnDamageDealt.AddDynamic(this, &AWukongCharacter::OnDamageDealtToEnemy);
    }

    // 设置玩家阵营（确保敌人 AI 能识别玩家为敌对目标）
    if (TeamComponent)
    {
        TeamComponent->SetTeam(ETeam::Player);
        UE_LOG(LogTemp, Log, TEXT("[Wukong] TeamComponent set to Player team"));
    }

    // 创建并初始化玩家 HUD
    if (PlayerHUDClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PlayerHUD = CreateWidget<UPlayerHUDWidget>(PC, PlayerHUDClass);
            if (PlayerHUD)
            {
                PlayerHUD->AddToViewport();
                PlayerHUD->InitializeHUD(this);
                UE_LOG(LogTemp, Log, TEXT("[Wukong] PlayerHUD created and initialized"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Wukong] PlayerHUDClass not set! Set it in Blueprint."));
    }

    // 初始化交互系统
    NearbyNPC = nullptr;
    InteractionPromptWidget = nullptr;
    InteractionCheckTimer = 0.0f;

    // 初始化变身系统
    bIsTransformed = false;
    ButterflyPawnInstance = nullptr;
    // 初始化对话状态
    bIsInDialogue = false;
    CurrentDialogueNPC = nullptr;
}

// 每帧都调用
void AWukongCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新状态机
    UpdateState(DeltaTime);

    // 更新冷却
    UpdateCooldowns(DeltaTime);

    // 体力更新由StaminaComponent自己处理

    // 检测附近的NPC
    InteractionCheckTimer += DeltaTime;
    if (InteractionCheckTimer >= InteractionCheckInterval)
    {
        InteractionCheckTimer = 0.0f;
        CheckForNearbyNPC();
    }

    // 对话中检测距离，超出则自动结束对话
    if (bIsInDialogue)
    {
        CheckDialogueDistance();
    }

    // 更新攻击冷却计时器
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // 更新InvincibilityTimer （无敌时间）
    if (bIsInvincible && InvincibilityTimer > 0.0f)
    {
        InvincibilityTimer -= DeltaTime;
        if (InvincibilityTimer <= 0.0f)
        {
            bIsInvincible = false;
        }
    }

    // 锁定目标时角色面向目标
    UpdateFacingTarget(DeltaTime);
}

// 把蓝图里配置的各个 InputAction 资产在运行时绑定到角色的对应方法上，确保按键触发后调用正确函数
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
        
        // 绑定dodge Action
        if (DodgeAction)
        {
            EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AWukongCharacter::OnDodgePressed);
            UE_LOG(LogTemp, Warning, TEXT("  Bound DodgeAction to OnDodgePressed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  DodgeAction is NULL! Dodge will not work!"));
        }

        // 绑定攻击Action
        if (AttackAction)
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformAttack);
            UE_LOG(LogTemp, Warning, TEXT("  Bound AttackAction to PerformAttack"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  AttackAction is NULL! Attack will not work!"));
        }

        // 绑定重击Action
        if (HeavyAttackAction)
        {
            EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformHeavyAttack);
            UE_LOG(LogTemp, Warning, TEXT("  Bound HeavyAttackAction to PerformHeavyAttack"));
        }

        // 绑定立棍Action
        if (PoleStanceAction)
        {
            EnhancedInputComponent->BindAction(PoleStanceAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformPoleStance);
            UE_LOG(LogTemp, Warning, TEXT("  Bound PoleStanceAction to PerformPoleStance"));
        }

        // 绑定甩花棍Action
        if (StaffSpinAction)
        {
            EnhancedInputComponent->BindAction(StaffSpinAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformStaffSpin);
            EnhancedInputComponent->BindAction(StaffSpinAction, ETriggerEvent::Completed, this, &AWukongCharacter::PerformStaffSpin); // Handle release if needed
            UE_LOG(LogTemp, Warning, TEXT("  Bound StaffSpinAction to PerformStaffSpin"));
        }

        // 绑定使用道具Action
        if (UseItemAction)
        {
            EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &AWukongCharacter::UseItem);
            UE_LOG(LogTemp, Warning, TEXT("  Bound UseItemAction to UseItem"));
        }

        // 绑定影分身技能Action（按1）
        if (ShadowCloneAction)
        {
            EnhancedInputComponent->BindAction(ShadowCloneAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformShadowClone);
            UE_LOG(LogTemp, Warning, TEXT("  Bound ShadowCloneAction to PerformShadowClone"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  ShadowCloneAction is NULL! Shadow Clone (Key 1) will not work! Assign IA_ShadowClone in BP_Wukong."));
        }

        // 绑定定身术技能Action（按2）
        if (FreezeSpellAction)
        {
            EnhancedInputComponent->BindAction(FreezeSpellAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformFreezeSpell);
            UE_LOG(LogTemp, Warning, TEXT("  Bound FreezeSpellAction to PerformFreezeSpell"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  FreezeSpellAction is NULL! Freeze Spell (Key 2) will not work! Assign IA_FreezeSpell in BP_Wukong."));
        }

        // 绑定冲刺Action
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

        // 绑定视角锁定Action
        if (LockOnAction)
        {
            EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AWukongCharacter::OnLockOnPressed);
            UE_LOG(LogTemp, Warning, TEXT("  Bound LockOnAction to OnLockOnPressed"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  LockOnAction is NULL - Lock-on disabled"));
        }

        // 绑定切换视角锁定对象Action
        if (SwitchTargetAction)
        {
            EnhancedInputComponent->BindAction(SwitchTargetAction, ETriggerEvent::Triggered, this, &AWukongCharacter::OnSwitchTarget);
            UE_LOG(LogTemp, Warning, TEXT("  Bound SwitchTargetAction to OnSwitchTarget"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  SwitchTargetAction is NULL - Target switching disabled"));
        }

        // 绑定交互Action
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AWukongCharacter::OnInteract);
            UE_LOG(LogTemp, Warning, TEXT("  Bound InteractAction (E) to OnInteract"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  InteractAction is NULL - Interaction disabled"));
        }

        // 绑定变身术Action（按3）
        if (TransformAction)
        {
            EnhancedInputComponent->BindAction(TransformAction, ETriggerEvent::Started, this, &AWukongCharacter::PerformTransform);
            UE_LOG(LogTemp, Warning, TEXT("  Bound TransformAction to PerformTransform"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  TransformAction is NULL! Transform (Key 3) will not work! Assign IA_Transform in BP_Wukong."));
        }

        EnhancedInputComponent->BindAction(TempleAction, ETriggerEvent::Triggered,
        this, &AWukongCharacter::OnTempleInteract);
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

    // 如果已经在攻击中，加入输入缓冲（用于Combo）
    if (CurrentState == EWukongState::Attacking)
    {
        InputBuffer.Add(TEXT("Attack"));
        return;
    }

    // 执行攻击
    PerformAttack();
}



// 处理输入
void AWukongCharacter::OnDodgePressed()
{
    UE_LOG(LogTemp, Warning, TEXT("OnDodgePressed() called! CurrentState=%d"), (int32)CurrentState);

    if (CurrentState == EWukongState::Attacking || 
        CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead ||
        bIsInDialogue)
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
    // 限制这几个情况下的攻击
    if (CurrentState == EWukongState::Dodging || 
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead || 
        bIsInDialogue)
    {
        return;
    }

    // 先缓冲
    InputBuffer.Add(TEXT("Attack"));

    // 如果没有处于攻击状态，就立即调用刚刚存在缓冲里面的攻击
    if (CurrentState != EWukongState::Attacking)
    {
        ProcessInputBuffer();
    }
    // 这是为了提高手感，让攻击在合适的时机接续
}

void AWukongCharacter::OnSprintStarted()
{
    if (CurrentState == EWukongState::Attacking || 
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun ||
        CurrentState == EWukongState::Dead || 
        bIsInDialogue)
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

float AWukongCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 调用父类逻辑
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 将通用伤害转发给我们的自定义伤害处理函数
    // 注意：EventInstigator 是 Controller，DamageCauser 是造成伤害的 Actor (如 Projectile)
    // ReceiveDamage 期望的是 DamageInstigator (通常是 Enemy 本身)
    // 如果 DamageCauser 是 Projectile，它的 Owner 通常是 Enemy
    AActor* InstigatorActor = DamageCauser;
    if (DamageCauser && DamageCauser->GetOwner())
    {
        InstigatorActor = DamageCauser->GetOwner();
    }
    
    ReceiveDamage(ActualDamage, InstigatorActor);

    return ActualDamage;
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

    // 调用刚刚存在缓冲区里面的Attack
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

        // 计算连击消耗的时间
        float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
        float ComboResetTime = CombatComponent ? CombatComponent->ComboResetTime : 1.0f;
        if (TimeSinceLastAttack > ComboResetTime)
        {
            ResetCombo();
        }

        // 返回合适的状态
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
    // 角色死亡什么都不需要做
}

void AWukongCharacter::PerformAttack()
{
    // 检查死亡状态 - 死亡后不能攻击
    if (CurrentState == EWukongState::Dead)
    {
        return;
    }

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
    // 死亡状态下不处理任何输入缓冲
    if (CurrentState == EWukongState::Dead)
    {
        InputBuffer.Empty();
        return;
    }

    if (InputBuffer.Num() == 0)
    {
        return;
    }

    // 清空缓冲
    float CurrentTime = GetWorld()->GetTimeSeconds();
    InputBuffer.RemoveAll([CurrentTime, this](const FString& Input) {
        return (CurrentTime - LastAttackTime) > InputBufferTime;
    });

    // 执行缓冲区中的第一次攻击
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

    // 播放闪避音效
    PlayDodgeSound();

    ChangeState(EWukongState::Dodging);

    // Determine dodge direction relative to actor
    FVector InputDirection = GetMovementInputDirection();
    UAnimMontage* MontageToPlay = DodgeFwdMontage; // Default to forward

    if (!InputDirection.IsNearlyZero())
    {
        DodgeDirection = InputDirection;
        
        // 计算向量点积以判定方向
        FVector ActorForward = GetActorForwardVector();
        FVector ActorRight = GetActorRightVector();
        
        float ForwardDot = FVector::DotProduct(ActorForward, InputDirection);
        float RightDot = FVector::DotProduct(ActorRight, InputDirection);

        if (ForwardDot > 0.707f)
        {
            MontageToPlay = DodgeFwdMontage;
        }
        else if (ForwardDot < -0.707f)
        {
            MontageToPlay = DodgeBwdMontage;
        }
        else if (RightDot > 0.0f)
        {
            MontageToPlay = DodgeRightMontage;
        }
        else
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

    if (MontageToPlay)
    {
        PlayMontage(MontageToPlay);
    }
    else
    {
        if (DodgeAnimation)
        {
             PlayAnimationAsMontageDynamic(DodgeAnimation, FName("DefaultSlot"), 1.0f);
        }
    }
}

void AWukongCharacter::PerformHeavyAttack()
{
    // 死亡、翻滚、硬直状态、对话下不能重击
    if (CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun ||
        bIsInDialogue)
    {
        return;
    }

    // 如果已经在攻击中，检查当前动画进度
    // 只有动画播放超过一定比例后才允许再次重击
    if (CurrentState == EWukongState::Attacking)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
            {
                float CurrentPos = AnimInstance->Montage_GetPosition(CurrentMontage);
                float TotalLength = CurrentMontage->GetPlayLength();
                
                // 动画播放未超过 70% 时，忽略重击输入
                const float HeavyAttackWindowThreshold = 0.7f;
                if (TotalLength > 0.0f && (CurrentPos / TotalLength) < HeavyAttackWindowThreshold)
                {
                    UE_LOG(LogTemp, Log, TEXT("PerformHeavyAttack: Blocked - animation at %.1f%%, need %.1f%%"), 
                        (CurrentPos / TotalLength) * 100.0f, HeavyAttackWindowThreshold * 100.0f);
                    return;
                }
            }
        }
    }

    // 检查是否有足够体力重击
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->HeavyAttackStaminaCost))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformHeavyAttack: Not enough stamina!"));
        return;
    }

    if (HeavyAttackMontage)
    {
        // 停止当前正在播放的蒙太奇（如果有的话）
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->StopAllMontages(0.1f);
        }
        
        // 消耗体力
        StaminaComponent->ConsumeStamina(StaminaComponent->HeavyAttackStaminaCost);
        
        ChangeState(EWukongState::Attacking);
        PlayMontage(HeavyAttackMontage);
    }
}

void AWukongCharacter::PerformStaffSpin()
{
    // 死亡、翻滚、硬直、对话状态下不能使用棍花
    if (CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun || 
        bIsInDialogue)
    {
        return;
    }

    // 如果已经在攻击中，检查当前动画进度
    if (CurrentState == EWukongState::Attacking)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
            {
                float CurrentPos = AnimInstance->Montage_GetPosition(CurrentMontage);
                float TotalLength = CurrentMontage->GetPlayLength();
                
                const float SkillWindowThreshold = 0.7f;
                if (TotalLength > 0.0f && (CurrentPos / TotalLength) < SkillWindowThreshold)
                {
                    return;
                }
            }
        }
    }

    // 检查是否有足够体力使用棍花
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->StaffSpinStaminaCost))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformStaffSpin: Not enough stamina!"));
        return;
    }

    if (StaffSpinMontage)
    {
        // 停止当前正在播放的蒙太奇
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->StopAllMontages(0.1f);
        }
        
        // 消耗体力
        StaminaComponent->ConsumeStamina(StaminaComponent->StaffSpinStaminaCost);
        
        ChangeState(EWukongState::Attacking); 
        PlayMontage(StaffSpinMontage);
    }
}

void AWukongCharacter::PerformPoleStance()
{
    // 死亡、翻滚、硬直、对话状态下不能使用立棍法
    if (CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun || 
        bIsInDialogue)
    {
        return;
    }

    // 如果已经在攻击中，检查当前动画进度
    if (CurrentState == EWukongState::Attacking)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
            {
                float CurrentPos = AnimInstance->Montage_GetPosition(CurrentMontage);
                float TotalLength = CurrentMontage->GetPlayLength();
                
                const float SkillWindowThreshold = 0.7f;
                if (TotalLength > 0.0f && (CurrentPos / TotalLength) < SkillWindowThreshold)
                {
                    return;
                }
            }
        }
    }

    // 检查是否有足够体力使用立棍法
    if (!StaminaComponent || !StaminaComponent->HasEnoughStamina(StaminaComponent->PoleStanceStaminaCost))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformPoleStance: Not enough stamina!"));
        return;
    }

    if (PoleStanceMontage)
    {
        // 停止当前正在播放的蒙太奇
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->StopAllMontages(0.1f);
        }
        
        // 消耗体力
        StaminaComponent->ConsumeStamina(StaminaComponent->PoleStanceStaminaCost);
        
        ChangeState(EWukongState::Attacking);
        PlayMontage(PoleStanceMontage);
    }
}

void AWukongCharacter::UseItem()
{
    // 对话中禁止使用道具
    if (bIsInDialogue)
    {
        return;
    }

    if (DrinkGourdMontage)
    {
        PlayMontage(DrinkGourdMontage);
    }
}

void AWukongCharacter::PerformShadowClone()
{
    UE_LOG(LogTemp, Warning, TEXT(">>> PerformShadowClone() CALLED! CurrentState=%d"), (int32)CurrentState);

    // 死亡、翻滚、硬直、对话状态下不能使用影分身
    if (CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun || 
        bIsInDialogue)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformShadowClone: Blocked by state"));
        return;
    }

    // 检查冷却
    if (IsCooldownActive(TEXT("ShadowClone")))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformShadowClone: On cooldown"));
        return;
    }

    // 检查是否设置了分身类
    if (!CloneClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformShadowClone: CloneClass not set! Set it in BP_Wukong blueprint."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PerformShadowClone: Summoning %d clones!"), CloneCount);

    // 开始冷却
    StartCooldown(TEXT("ShadowClone"), ShadowCloneCooldown);

    // 播放召唤动画（如果有的话）
    if (ShadowCloneMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(ShadowCloneMontage, 1.0f);
        }
    }

    // 生成分身
    FVector PlayerLocation = GetActorLocation();
    FRotator PlayerRotation = GetActorRotation();

    for (int32 i = 0; i < CloneCount; i++)
    {
        // 计算生成位置（围绕玩家均匀分布）
        float Angle = (360.0f / CloneCount) * i + 90.0f; // 从两侧开始
        float AngleRad = FMath::DegreesToRadians(Angle);
        
        FVector SpawnOffset = FVector(
            FMath::Cos(AngleRad) * CloneSpawnDistance,
            FMath::Sin(AngleRad) * CloneSpawnDistance,
            0.0f
        );
        
        FVector SpawnLocation = PlayerLocation + SpawnOffset;
        
        // 进行地面检测，确保分身生成在地面上
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 500.0f);
        FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 500.0f);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            // 找到地面，将生成点设置在地面上方一点
            SpawnLocation = HitResult.Location + FVector(0.0f, 0.0f, 90.0f); // 胶囊体半高
        }
        
        // 让分身面向玩家的朝向
        FRotator SpawnRotation = PlayerRotation;

        // 设置生成参数
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 生成分身
        AWukongClone* Clone = GetWorld()->SpawnActor<AWukongClone>(
            CloneClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (Clone)
        {
            // 初始化分身
            Clone->InitializeClone(this, CloneLifetime);
            UE_LOG(LogTemp, Log, TEXT("PerformShadowClone: Spawned clone %d at %s"), i + 1, *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PerformShadowClone: Failed to spawn clone %d"), i + 1);
        }
    }
}

// ========== 定身术实现 ==========

void AWukongCharacter::PerformFreezeSpell()
{
    UE_LOG(LogTemp, Warning, TEXT(">>> PerformFreezeSpell() CALLED! CurrentState=%d"), (int32)CurrentState);

    // 死亡、翻滚、硬直、对话状态下不能使用定身术
    if (CurrentState == EWukongState::Dead ||
        CurrentState == EWukongState::Dodging ||
        CurrentState == EWukongState::HitStun || 
        bIsInDialogue)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformFreezeSpell: Blocked by state"));
        return;
    }

    // 检查冷却
    if (IsCooldownActive(TEXT("FreezeSpell")))
    {
        UE_LOG(LogTemp, Log, TEXT("PerformFreezeSpell: On cooldown"));
        return;
    }

    // 检查是否有锁定目标
    if (!TargetingComponent || !TargetingComponent->IsTargeting())
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformFreezeSpell: No locked target! Lock onto an enemy first (Mouse Middle Button)."));
        return;
    }

    
    // 获取锁定的目标
    AActor* LockedTarget = TargetingComponent->GetLockedTarget();
    if (!LockedTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformFreezeSpell: Locked target is invalid!"));
        return;
    }

    // 检查目标是否是敌人基类
    AEnemyBase* TargetEnemy = Cast<AEnemyBase>(LockedTarget);
    if (!TargetEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformFreezeSpell: Target is not an enemy! Cannot freeze."));
        return;
    }

    // 检查敌人是否已死亡
    if (TargetEnemy->IsDead())
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformFreezeSpell: Target is already dead!"));
        return;
    }

    // 检查敌人是否已被定身
    if (TargetEnemy->IsFrozen())
    {
        UE_LOG(LogTemp, Log, TEXT("PerformFreezeSpell: Target is already frozen!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PerformFreezeSpell: Casting freeze on %s for %.1f seconds!"), *TargetEnemy->GetName(), FreezeSpellDuration);

    // 开始冷却
    StartCooldown(TEXT("FreezeSpell"), FreezeSpellCooldown);

    // 播放施法动画（如果有的话）
    if (FreezeSpellMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Play(FreezeSpellMontage, 1.0f);
        }
    }

    // 对目标敌人施加定身效果
    TargetEnemy->ApplyFreeze(FreezeSpellDuration);
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

    // 基于剩余时间，计算翻滚的速度
    float DodgeSpeed = DodgeDistance / DodgeDuration;
    FVector DodgeVelocity = DodgeDirection * DodgeSpeed * DeltaTime;

    // 执行翻滚动作
    FHitResult HitResult;
    AddActorWorldOffset(DodgeVelocity, true, &HitResult);
}

// Cooldown Management
bool AWukongCharacter::IsCooldownActive(const FString& CooldownName) const
{
    const float* CooldownTime = CooldownMap.Find(CooldownName);
    return (CooldownTime != nullptr && *CooldownTime > 0.0f);
}

float AWukongCharacter::GetTransformCooldownRemaining() const
{
    const float* CooldownTime = CooldownMap.Find(TEXT("Transform"));
    return (CooldownTime != nullptr) ? *CooldownTime : 0.0f;
}

void AWukongCharacter::StartCooldown(const FString& CooldownName, float Duration)
{
    CooldownMap.Add(CooldownName, Duration);

    // 通知 HUD 更新技能冷却显示
    if (PlayerHUD)
    {
        // 技能名称到槽位索引的映射（按照按键顺序）
        // 槽位0: 分身术 (按键1)
        // 槽位1: 定身术 (按键2)
        // 槽位2: 变身术 (按键3)
        // 槽位3: 法术 (按键4)
        if (CooldownName == TEXT("ShadowClone"))
        {
            PlayerHUD->TriggerSkillCooldown(0, Duration);
        }
        else if (CooldownName == TEXT("FreezeSpell"))
        {
            PlayerHUD->TriggerSkillCooldown(1, Duration);
        }
        else if (CooldownName == TEXT("Transform"))
        {
            PlayerHUD->TriggerSkillCooldown(2, Duration);
        }
        // 可以继续添加更多技能映射
    }
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

// ========== 连击计数回调 ==========
void AWukongCharacter::OnDamageDealtToEnemy(float Damage, AActor* Target, bool bIsCritical)
{
    // 增加连击计数
    HitComboCount++;

    // 更新 HUD 连击显示
    if (PlayerHUD)
    {
        PlayerHUD->UpdateComboCount(HitComboCount);
    }

    // 触发战斗BGM切换（玩家攻击命中敌人时）
    if (AGameStateBase* GameState = GetWorld()->GetGameState())
    {
        if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
        {
            SceneComp->OnCombatInitiated();
        }
    }

    // 重置连击计时器（2秒内无命中则重置）
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(
            ComboResetTimerHandle,
            this,
            &AWukongCharacter::ResetHitCombo,
            2.0f,  // 2秒后重置
            false
        );
    }

    UE_LOG(LogTemp, Log, TEXT("OnDamageDealtToEnemy: Hit %s for %.1f damage, Combo: %d"),
        Target ? *Target->GetName() : TEXT("None"), Damage, HitComboCount);
}

void AWukongCharacter::ResetHitCombo()
{
    HitComboCount = 0;
    if (PlayerHUD)
    {
        PlayerHUD->UpdateComboCount(0);
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
    // 对话中禁止跳跃
    if (bIsInDialogue)
    {
        return false;
    }

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

    // 播放跳跃音效
    PlayJumpSound();

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
    // 防止重复调用
    if (CurrentState == EWukongState::Dead)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Die: Character dying..."));

    ChangeState(EWukongState::Dead);
    
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Die: No Mesh Component!"));
        return;
    }

    // 如果有死亡动画，直接让 Mesh 播放（不通过动画蓝图）
    if (DeathAnimation)
    {
        UE_LOG(LogTemp, Log, TEXT("Die: Playing death animation directly: %s"), *DeathAnimation->GetName());
        
        // 停止动画蓝图，切换到直接播放动画模式
        MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        MeshComp->PlayAnimation(DeathAnimation, false); // false = 不循环
        
        // 获取死亡动画时长
        float AnimDuration = DeathAnimation->GetPlayLength();
        UE_LOG(LogTemp, Log, TEXT("Die: Death animation duration=%f"), AnimDuration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Die: No DeathAnimation set in Blueprint!"));
        
        // 如果没有死亡动画，至少让角色停在当前帧
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            AnimInstance->StopAllMontages(0.0f);
        }
    }

    // 禁用角色碰撞（防止死亡后阻挡其他角色或继续被攻击）
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 禁用玩家输入
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
        UE_LOG(LogTemp, Log, TEXT("Die: Player input disabled"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Die: Character has died"));
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

    // 自动找一个可用的蒙太奇插槽，把纯动画序列即时包装成蒙太奇播放
    TArray<FName> SlotCandidates;
    if (!SlotName.IsNone())
    {
        SlotCandidates.Add(SlotName);
    }
    // 一些可能的slot名，纯粹防御性编程
    SlotCandidates.Add(FName("DefaultGroup.FullBody"));
    SlotCandidates.Add(FName("FullBody"));
    SlotCandidates.Add(FName("DefaultGroup.UpperBody"));
    SlotCandidates.Add(FName("UpperBody"));
    SlotCandidates.Add(FName("DefaultSlot"));
    SlotCandidates.Add(FName("Default"));

    for (const FName& CandidateSlot : SlotCandidates)
    {
        UAnimMontage* TempMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(
            AnimSequence,
            CandidateSlot,
            0.12f, // 渐入时间
            0.12f, // 淡出时间
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
// ========== 目标锁定输入处理 ==========

void AWukongCharacter::OnLockOnPressed()
{
    UE_LOG(LogTemp, Log, TEXT("OnLockOnPressed() called"));
    
    if (TargetingComponent)
    {
        TargetingComponent->ToggleLockOn();
    }
}

void AWukongCharacter::OnSwitchTarget(const FInputActionValue& Value)
{
    if (!TargetingComponent || !TargetingComponent->IsTargeting())
    {
        return;
    }

    // 获取鼠标滚轮值，正值向右切换，负值向左切换
    float ScrollValue = Value.Get<float>();
    
    if (FMath::Abs(ScrollValue) > 0.1f)
    {
        bool bSwitchRight = ScrollValue > 0.0f;
        TargetingComponent->SwitchTarget(bSwitchRight);
        UE_LOG(LogTemp, Log, TEXT("SwitchTarget: %s"), bSwitchRight ? TEXT("Right") : TEXT("Left"));
    }
}

void AWukongCharacter::UpdateFacingTarget(float DeltaTime)
{
    // 只在锁定状态下处理
    if (!TargetingComponent || !TargetingComponent->IsTargeting())
    {
        return;
    }

    // 翻滚、死亡状态不调整朝向
    if (CurrentState == EWukongState::Dodging || CurrentState == EWukongState::Dead)
    {
        return;
    }

    AActor* Target = TargetingComponent->GetLockedTarget();
    if (!Target)
    {
        return;
    }

    // 计算朝向目标的方向（只在水平面上）
    FVector OwnerLocation = GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal2D();

    if (DirectionToTarget.IsNearlyZero())
    {
        return;
    }

    // 计算目标旋转
    FRotator TargetRotation = DirectionToTarget.Rotation();
    FRotator CurrentRotation = GetActorRotation();

    // 平滑插值到目标朝向
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 10.0f);
    NewRotation.Pitch = 0.0f;
    NewRotation.Roll = 0.0f;

    SetActorRotation(NewRotation);
}

// ========== NPC交互系统实现 ==========

void AWukongCharacter::CheckForNearbyNPC()
{
	// 使用OverlapMultiByObjectType检测所有Actor（避免碰撞通道问题）
	TArray<FOverlapResult> OverlapResults;
	FVector StartLocation = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionDistance);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	// 使用WorldDynamic通道检测所有动态物体
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		StartLocation,
		FQuat::Identity,
		ObjectQueryParams,
		Sphere,
		QueryParams
	);

	ANPCCharacter* ClosestNPC = nullptr;
	float ClosestDistance = InteractionDistance;

	if (bHit)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Interaction] Found %d overlapping actors"), OverlapResults.Num());
		
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (ANPCCharacter* NPC = Cast<ANPCCharacter>(Overlap.GetActor()))
			{
				// 只在Verbose级别输出,避免刷屏
				UE_LOG(LogTemp, Verbose, TEXT("[Interaction] Checking NPC: %s, CanInteract: %d"), 
					*NPC->GetName(), NPC->CanBeInteractedWith());
				
				if (NPC->CanBeInteractedWith())
				{
					float Distance = FVector::Dist(StartLocation, NPC->GetActorLocation());
					if (Distance < ClosestDistance)
					{
						ClosestNPC = NPC;
						ClosestDistance = Distance;
					}
				}
			}
		}
	}

	// 更新当前NPC
	if (ClosestNPC != NearbyNPC)
	{
		NearbyNPC = ClosestNPC;
		
		if (NearbyNPC)
		{
			UE_LOG(LogTemp, Log, TEXT("[Interaction] NPC in range: %s"), *NearbyNPC->GetName());
			ShowInteractionPrompt();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Interaction] NPC out of range"));
			HideInteractionPrompt();
		}
	}
}

void AWukongCharacter::ShowInteractionPrompt()
{
	if (!InteractionPromptWidget)
	{
		if (InteractionPromptWidgetClass)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(PC, InteractionPromptWidgetClass);
				if (InteractionPromptWidget)
				{
					InteractionPromptWidget->AddToViewport(100); // 高优先级
					UE_LOG(LogTemp, Log, TEXT("[Interaction] Created InteractionPromptWidget"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[Interaction] Failed to create InteractionPromptWidget!"));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Interaction] InteractionPromptWidgetClass is NULL! Set it in BP_Wukong!"));
		}
	}

	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->ShowPrompt(FText::FromString(TEXT("按 [E] 对话")));
		UE_LOG(LogTemp, Log, TEXT("[Interaction] Showing prompt"));
	}
}

void AWukongCharacter::HideInteractionPrompt()
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->HidePrompt();
	}
}

void AWukongCharacter::OnInteract()
{
	UE_LOG(LogTemp, Log, TEXT("[Interaction] OnInteract called, NearbyNPC: %s"), 
		NearbyNPC ? *NearbyNPC->GetName() : TEXT("NULL"));
	
	if (NearbyNPC && NearbyNPC->CanBeInteractedWith())
	{
		// 检查是否正在对话中
		if (NearbyNPC->DialogueComponent && NearbyNPC->DialogueComponent->IsDialoguePlaying())
		{
			UE_LOG(LogTemp, Log, TEXT("[Interaction] Dialogue in progress, calling NextDialogue"));
			// 对话中，按E继续下一句
			NearbyNPC->DialogueComponent->NextDialogue();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Interaction] Starting new dialogue"));
			// 不在对话中，开始新对话
			HideInteractionPrompt();
			NearbyNPC->StartDialogue();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interaction] No nearby NPC or cannot interact!"));
	}
}

// ========== 音效系统 ==========

void AWukongCharacter::PlayFootstepSound()
{
	if (FootstepSound)
	{
		// 根据是否冲刺决定音量
		float Volume = bIsSprinting ? SprintFootstepVolume : WalkFootstepVolume;
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation(), Volume);
	}
}

void AWukongCharacter::PlayAttackSound()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation(), AttackSoundVolume);
	}
}

void AWukongCharacter::PlayDodgeSound()
{
	if (DodgeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, GetActorLocation(), DodgeSoundVolume);
	}
}

void AWukongCharacter::PlayJumpSound()
{
	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation(), JumpSoundVolume);
	}
}
// ========== 变身术系统 ==========

void AWukongCharacter::PerformTransform()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> PerformTransform() CALLED! bIsTransformed=%d"), bIsTransformed);

	// 如果已经变身了，不能再次变身
	if (bIsTransformed)
	{
		UE_LOG(LogTemp, Log, TEXT("PerformTransform: Already transformed"));
		return;
	}

	// 检查冷却（使用统一的冷却系统）
	if (IsCooldownActive(TEXT("Transform")))
	{
		UE_LOG(LogTemp, Log, TEXT("PerformTransform: On cooldown"));
		return;
	}

	// 检查状态 - 死亡、翻滚、硬直、攻击中不能变身
	if (CurrentState == EWukongState::Dead ||
		CurrentState == EWukongState::Dodging ||
		CurrentState == EWukongState::HitStun ||
		CurrentState == EWukongState::Attacking)
	{
		UE_LOG(LogTemp, Log, TEXT("PerformTransform: Blocked by state"));
		return;
	}

	// 对话中不能变身
	if (bIsInDialogue)
	{
		UE_LOG(LogTemp, Log, TEXT("PerformTransform: Blocked by dialogue"));
		return;
	}

	// 执行变身
	TransformToButterfly();
}

void AWukongCharacter::TransformToButterfly()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> TransformToButterfly() - Starting transformation!"));

	// 解除锁定状态（变身后不应该保持锁定）
	if (TargetingComponent && TargetingComponent->IsTargeting())
	{
		TargetingComponent->ClearTarget();
		UE_LOG(LogTemp, Log, TEXT("TransformToButterfly: Cleared target lock"));
	}

	// 检查蝴蝶Pawn类是否配置
	if (!ButterflyPawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TransformToButterfly: ButterflyPawnClass is NULL! Set it in BP_Wukong."));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TransformToButterfly: No PlayerController!"));
		return;
	}

	// 在悟空位置生成蝴蝶
	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f); // 稍微抬高一点
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ButterflyPawnInstance = GetWorld()->SpawnActor<APawn>(
		ButterflyPawnClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!ButterflyPawnInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("TransformToButterfly: Failed to spawn butterfly!"));
		return;
	}

	// 标记变身状态
	bIsTransformed = true;

	// 保存悟空当前位置
	PreTransformLocation = GetActorLocation();

	// 隐藏悟空并禁用碰撞
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 禁用悟空的移动
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	// 将悟空附加到蝴蝶上（这样悟空会跟随蝴蝶移动，变回时已经在正确位置）
	// 敌人AI会通过检查bIsTransformed来忽略悟空
	AttachToActor(ButterflyPawnInstance, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 清除所有敌人的仇恨（让敌人"无视"蝴蝶）
	ClearAllEnemyAggro();

	// 控制器切换到蝴蝶
	PC->Possess(ButterflyPawnInstance);

	// 初始化蝴蝶Pawn的变身计时器（由蝴蝶自己管理计时，因为悟空被UnPossess后计时器可能不可靠）
	if (AButterflyPawn* ButterflyPawn = Cast<AButterflyPawn>(ButterflyPawnInstance))
	{
		ButterflyPawn->InitializeTransform(this, TransformDuration);
		UE_LOG(LogTemp, Log, TEXT("TransformToButterfly: Initialized butterfly timer with duration=%.1f"), TransformDuration);
	}

	UE_LOG(LogTemp, Warning, TEXT("TransformToButterfly: SUCCESS! Duration=%.1f seconds"), TransformDuration);
}

void AWukongCharacter::TransformBackToWukong()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> TransformBackToWukong() - Reverting transformation!"));

	if (!bIsTransformed)
	{
		UE_LOG(LogTemp, Warning, TEXT("TransformBackToWukong: Not transformed, skipping"));
		return;
	}

	APlayerController* PC = nullptr;
	
	// 获取控制器（当前控制蝴蝶的）
	if (ButterflyPawnInstance)
	{
		PC = Cast<APlayerController>(ButterflyPawnInstance->GetController());
	}

	if (!PC)
	{
		// 尝试获取玩家0的控制器
		PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	}

	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TransformBackToWukong: No PlayerController found!"));
		return;
	}

	// 解除悟空与蝴蝶的附加关系
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	// 悟空现在已经在蝴蝶位置了（因为一直附加着）
	FVector CurrentLocation = GetActorLocation();
	FRotator FinalRotation = FRotator::ZeroRotator;
	
	if (ButterflyPawnInstance)
	{
		FinalRotation = ButterflyPawnInstance->GetActorRotation();
		FinalRotation.Pitch = 0.0f;
		FinalRotation.Roll = 0.0f;
		
		UE_LOG(LogTemp, Warning, TEXT("TransformBackToWukong: Wukong location after detach: X=%.1f, Y=%.1f, Z=%.1f"),
			CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z);
		
		// 销毁蝴蝶
		ButterflyPawnInstance->Destroy();
		ButterflyPawnInstance = nullptr;
	}

	// 设置旋转
	SetActorRotation(FinalRotation);

	// 显示悟空并恢复碰撞
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 恢复移动模式
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Falling);
	}

	// 控制器切回悟空
	PC->Possess(this);

	// 重置状态
	bIsTransformed = false;
	ChangeState(EWukongState::Idle);

	// 启动冷却（同时触发UI更新）
	StartCooldown(TEXT("Transform"), TransformCooldown);

	// 清除计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TransformTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("TransformBackToWukong: SUCCESS! Cooldown=%.1f seconds"), TransformCooldown);
}

void AWukongCharacter::OnTransformDurationEnd()
{
	UE_LOG(LogTemp, Warning, TEXT(">>> OnTransformDurationEnd() - Transform duration expired!"));
	TransformBackToWukong();
}
// ========== 对话系统 ==========

void AWukongCharacter::SetInDialogue(bool bInDialogue)
{
	bIsInDialogue = bInDialogue;
	
	if (bInDialogue)
	{
		// 记录当前对话的NPC
		CurrentDialogueNPC = NearbyNPC;
		UE_LOG(LogTemp, Log, TEXT("[Dialogue] Entered dialogue mode with %s"), 
			CurrentDialogueNPC ? *CurrentDialogueNPC->GetName() : TEXT("NULL"));
	}
	else
	{
		// 对话结束，清空记录
		CurrentDialogueNPC = nullptr;
		UE_LOG(LogTemp, Log, TEXT("[Dialogue] Exited dialogue mode"));
	}
}

void AWukongCharacter::CheckDialogueDistance()
{
	// 如果没有在对话中或没有对话NPC，直接返回
	if (!bIsInDialogue || !CurrentDialogueNPC)
	{
		return;
	}

	// 计算与NPC的距离
	float Distance = FVector::Dist(GetActorLocation(), CurrentDialogueNPC->GetActorLocation());
	
	// 超过阈值，自动结束对话
	if (Distance > DialogueBreakDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Distance %.1f > %.1f, auto-ending dialogue"), 
			Distance, DialogueBreakDistance);
		
		// 调用NPC的DialogueComponent结束对话
		if (CurrentDialogueNPC->DialogueComponent)
		{
			CurrentDialogueNPC->DialogueComponent->EndDialogue();
		}
		
		// 重置状态（EndDialogue会调用SetInDialogue(false)，但这里double check）
		bIsInDialogue = false;
		CurrentDialogueNPC = nullptr;
	}
}

void AWukongCharacter::ClearAllEnemyAggro()
{
	// 获取所有敌人并清除他们的仇恨
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

	for (AActor* Actor : FoundEnemies)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
		{
			Enemy->ClearCombatTarget();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Transform] Cleared aggro from %d enemies"), FoundEnemies.Num());
}

	// ========== 土地庙交互系统 ==========
void AWukongCharacter::OnTempleInteract()
{
    if (!CurrentInteractable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Temple] No CurrentInteractable"));
        return;
    }

    if (CurrentInteractable->Implements<UInteractInterface>())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Temple] Q Interact with %s"),
            *CurrentInteractable->GetName());

        IInteractInterface::Execute_OnInteract(CurrentInteractable, this);
    }
}

