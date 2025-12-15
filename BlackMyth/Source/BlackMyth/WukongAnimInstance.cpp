// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongAnimInstance.h"
#include "WukongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UWukongAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    RefreshOwningCharacter();
}

void UWukongAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // 确保引用有效（支持悟空和分身等通用角色）
    if (!CachedMovementComponent.IsValid())
    {
        RefreshOwningCharacter();
        if (!CachedMovementComponent.IsValid())
        {
            return;
        }
    }

    // 更新所有动画变量
    UpdateMovementVariables();
    
    // 战斗变量只有悟空角色才有
    if (CachedWukongCharacter.IsValid())
    {
        UpdateCombatVariables();
    }
}

void UWukongAnimInstance::RefreshOwningCharacter()
{
    APawn* OwningPawn = TryGetPawnOwner();
    CachedWukongCharacter = Cast<AWukongCharacter>(OwningPawn);
    
    if (CachedWukongCharacter.IsValid())
    {
        CachedMovementComponent = CachedWukongCharacter->GetCharacterMovement();
        // 设置 Paragon 兼容的 Character 引用
        Character = CachedWukongCharacter.Get();
    }
    else if (ACharacter* GenericCharacter = Cast<ACharacter>(OwningPawn))
    {
        // 支持非 WukongCharacter 的通用角色（如分身 WukongClone）
        // 这样动画蓝图可以正常工作
        CachedMovementComponent = GenericCharacter->GetCharacterMovement();
        Character = GenericCharacter;
    }
}

void UWukongAnimInstance::UpdateMovementVariables()
{
    const AWukongCharacter* Wukong = CachedWukongCharacter.Get();
    const UCharacterMovementComponent* MovementComp = CachedMovementComponent.Get();
    
    // 如果没有移动组件，重置所有变量
    if (!MovementComp)
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsGrounded = true;
        LocomotionPlayRate = 1.0f;
        bIsAccelerating = false;
        bIsWalking = false;
        return;
    }

    // 从 Wukong Character 获取动画引用 (如果尚未获取，且是悟空角色)
    if (!IdleAnimation && Wukong)
    {
        IdleAnimation = Wukong->IdleAnimation;
        WalkAnimation = Wukong->WalkForwardAnimation;
        SprintAnimation = Wukong->SprintForwardAnimation;
    }

    // 获取速度
    const FVector Velocity = MovementComp->Velocity;
    FVector HorizontalVelocity = Velocity;
    HorizontalVelocity.Z = 0.0f;
    
    // 更新跳跃/下落状态（先更新这个，因为后面要用）
    const bool bCurrentlyFalling = MovementComp->IsFalling();
    bWasGrounded = bIsGrounded;
    bIsFalling = bCurrentlyFalling;
    bIsGrounded = !bCurrentlyFalling;
    
    // 更新速度和方向
    // 注意：物理上角色仍保持惯性（抛物线轨迹），但动画只在地面时播放走路
    // 空中时 Speed 设为0，这样 AnimBP 不会播放走路动画，但角色仍沿抛物线移动
    
    // 先获取实际水平速度（用于判断移动状态）
    const float ActualHorizontalSpeed = HorizontalVelocity.Size();
    
    // 获取角色引用（通用）
    ACharacter* OwnerCharacter = Character;
    
    if (bIsFalling)
    {
        // 空中时：动画系统的 Speed 设为0，防止播放走路/跑步动画
        // 但物理系统仍保持惯性速度（AirControl=0 只是不响应输入，不会清零速度）
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsWalking = false;
        bIsSprinting = false;
        bIsAccelerating = false;
        LocomotionPlayRate = 1.0f;
    }
    else
    {
        // 地面时：正常更新移动变量
        Speed = ActualHorizontalSpeed;
        
        // 使用通用 Character 引用计算方向
        if (OwnerCharacter)
        {
            Direction = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
        }
        else
        {
            Direction = 0.0f;
        }
        
        bIsMoving = Speed > 10.0f;
        
        // 更新冲刺状态（只有悟空角色才有冲刺）
        bIsSprinting = Wukong ? Wukong->IsSprinting() : (Speed > 450.0f);
        
        // 更新走路状态（移动但不冲刺）
        bIsWalking = bIsMoving && !bIsSprinting;

        // 是否正在加速（检查当前加速度，对应原蓝图中的 isAccelerating）
        const FVector CurrentAcceleration = MovementComp->GetCurrentAcceleration();
        bIsAccelerating = CurrentAcceleration.SizeSquared2D() > KINDA_SMALL_NUMBER;
        
        // 计算动画播放速率
        const float SprintSpeed = 600.0f;
        
        if (bIsSprinting)
        {
            LocomotionPlayRate = 1.0f;
        }
        else if (Speed > 10.0f)
        {
            LocomotionPlayRate = FMath::Clamp(Speed / SprintSpeed, 0.4f, 0.7f);
        }
        else
        {
            LocomotionPlayRate = 1.0f;
        }
    }
    
    // 检测刚刚起跳
    if (bIsFalling && bWasGrounded)
    {
        bJustJumped = true;
    }
    else if (bIsGrounded)
    {
        bJustJumped = false;
    }
    
    // 垂直速度
    VerticalVelocity = Velocity.Z;

    // ========== 计算 Locomotion State (状态机逻辑) ==========
    if (bIsFalling)
    {
        // 空中状态现在由 Montage 接管，这里不需要特殊处理
        // 或者可以保留一个 Idle 状态，防止 Montage 没播出来时 T-Pose
        LocomotionState = ELocomotionState::Idle;
    }
    else // 地面
    {
        // 简单的落地检测逻辑：如果上一帧是空中，这一帧是地面，可以短暂切到 JumpEnd
        // 但这里为了简化，直接根据速度判断
        
        if (Speed < 10.0f)
        {
            LocomotionState = ELocomotionState::Idle;
        }
        else if (Speed > 450.0f) // 跑步阈值
        {
            LocomotionState = ELocomotionState::Run;
        }
        else
        {
            LocomotionState = ELocomotionState::Walk;
        }
    }

    // ========== 状态转换逻辑 (Montage 触发) ==========
    // 检测 Run -> Walk 的转换，播放急停过渡动画
    if (LocomotionState != PreviousLocomotionState)
    {
        if (PreviousLocomotionState == ELocomotionState::Run && LocomotionState == ELocomotionState::Walk)
        {
            // 只有在地面且没有播放其他 Montage 时才播放
            if (!IsAnyMontagePlaying() && RunToWalkMontage)
            {
                Montage_Play(RunToWalkMontage);
            }
        }
        PreviousLocomotionState = LocomotionState;
    }

    // ===== 更新瞄准偏移变量（对应原蓝图 Set Roll Pitch and Yaw） =====
    // 这些变量用于 Aim Offset BlendSpace，让上半身跟随瞄准方向
    // 只有悟空角色才有控制旋转，分身使用角色朝向
    if (OwnerCharacter)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator ControlRotation = Wukong ? Wukong->GetControlRotation() : ActorRotation;
    
        // 计算控制器相对于角色的旋转差
        const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
        
        AimPitch = FMath::Clamp(DeltaRotation.Pitch, -90.0f, 90.0f);
        AimYaw = FMath::Clamp(DeltaRotation.Yaw, -90.0f, 90.0f);
        AimRoll = DeltaRotation.Roll;

        // 计算 Yaw 变化量（用于转向动画混合）
        const float CurrentYaw = ActorRotation.Yaw;
        YawDelta = UKismetMathLibrary::NormalizedDeltaRotator(
            FRotator(0.0f, CurrentYaw, 0.0f),
            FRotator(0.0f, PreviousYaw, 0.0f)
        ).Yaw;
        PreviousYaw = CurrentYaw;

        // ===== 同步 Paragon 兼容变量 =====
        isAccelerating = bIsAccelerating;
        Pitch = AimPitch;
        Yaw = AimYaw;
        Roll = AimRoll;
    }
}

void UWukongAnimInstance::UpdateCombatVariables()
{
    const AWukongCharacter* Wukong = CachedWukongCharacter.Get();
    if (!Wukong)
    {
        return;
    }

    // 从角色状态读取战斗信息
    const EWukongState CurrentState = Wukong->GetCurrentState();
    
    bIsAttacking = (CurrentState == EWukongState::Attacking);
    bIsDodging = (CurrentState == EWukongState::Dodging);
    bIsHitStunned = (CurrentState == EWukongState::HitStun);
    bIsDead = (CurrentState == EWukongState::Dead);
    
    // 获取连击索引
    ComboIndex = Wukong->GetComboIndex();
    
    // 计算是否可以过渡状态
    bCanTransition = !bIsAttacking && !bIsDodging && !bIsHitStunned && !bIsDead;

    // 检测是否正在播放 FullBody 动画
    // 对应原蓝图中的 isFullbody 变量 - 通过检测 FullBody Curve 值来判断
    // 如果 Montage 中有名为 "FullBody" 的曲线，其值大于 0 时表示全身动画
    bIsFullBody = GetCurveValue(FName("FullBody")) > 0.0f;

    // ===== 同步 Paragon 兼容变量 =====
    isFullbody = bIsFullBody;
}

float UWukongAnimInstance::CalculateDirection(const FVector& Velocity, const FRotator& BaseRotation) const
{
    if (Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }
    
    // 使用 Kismet 库计算方向
    return UKismetMathLibrary::NormalizedDeltaRotator(
        Velocity.ToOrientationRotator(),
        BaseRotation
    ).Yaw;
}

// ========== Animation Notify 回调 ==========

void UWukongAnimInstance::AnimNotify_AttackHitStart()
{
    // 这里可以通知角色开始检测攻击碰撞
    // TODO: 当实现 CombatComponent 时，在这里调用开始检测
    UE_LOG(LogTemp, Log, TEXT("Attack Hit Detection Started"));
}

void UWukongAnimInstance::AnimNotify_AttackHitEnd()
{
    // 停止攻击碰撞检测
    UE_LOG(LogTemp, Log, TEXT("Attack Hit Detection Ended"));
}

void UWukongAnimInstance::AnimNotify_ComboWindowStart()
{
    // 开始接受连击输入
    UE_LOG(LogTemp, Log, TEXT("Combo Window Opened"));
}

void UWukongAnimInstance::AnimNotify_ComboWindowEnd()
{
    // 连击输入窗口关闭
    UE_LOG(LogTemp, Log, TEXT("Combo Window Closed"));
}

