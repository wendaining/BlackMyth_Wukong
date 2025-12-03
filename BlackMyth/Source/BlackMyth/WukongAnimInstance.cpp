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

    // 确保引用有效
    if (!CachedWukongCharacter.IsValid())
    {
        RefreshOwningCharacter();
        if (!CachedWukongCharacter.IsValid())
        {
            return;
        }
    }

    // 更新所有动画变量
    UpdateMovementVariables();
    UpdateCombatVariables();
}

void UWukongAnimInstance::RefreshOwningCharacter()
{
    APawn* OwningPawn = TryGetPawnOwner();
    CachedWukongCharacter = Cast<AWukongCharacter>(OwningPawn);
    
    if (CachedWukongCharacter.IsValid())
    {
        CachedMovementComponent = CachedWukongCharacter->GetCharacterMovement();
    }
}

void UWukongAnimInstance::UpdateMovementVariables()
{
    const AWukongCharacter* Wukong = CachedWukongCharacter.Get();
    const UCharacterMovementComponent* MovementComp = CachedMovementComponent.Get();
    
    if (!Wukong || !MovementComp)
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsGrounded = true;
        LocomotionPlayRate = 1.0f;
        bIsAccelerating = false;
        return;
    }

    // 获取速度
    const FVector Velocity = MovementComp->Velocity;
    FVector HorizontalVelocity = Velocity;
    HorizontalVelocity.Z = 0.0f;
    
    // 更新速度和方向
    Speed = HorizontalVelocity.Size();
    Direction = CalculateDirection(Velocity, Wukong->GetActorRotation());
    bIsMoving = Speed > 10.0f;
    
    // 更新冲刺状态
    bIsSprinting = Wukong->IsSprinting();

    // 是否正在加速（检查当前加速度，对应原蓝图中的 isAccelerating）
    const FVector CurrentAcceleration = MovementComp->GetCurrentAcceleration();
    bIsAccelerating = CurrentAcceleration.SizeSquared2D() > KINDA_SMALL_NUMBER;
    
    // 计算动画播放速率
    // 走路时（不冲刺）播放速率较低，让动画看起来更慢
    // 冲刺时播放速率正常
    const float WalkSpeed = 300.0f;  // 与 WukongCharacter 中的 WalkSpeed 保持一致
    const float SprintSpeed = 600.0f; // 与 WukongCharacter 中的 SprintSpeed 保持一致
    
    if (bIsSprinting)
    {
        // 冲刺时正常播放
        LocomotionPlayRate = 1.0f;
    }
    else if (Speed > 10.0f)
    {
        // 走路时，根据速度比例降低播放速率
        // 这样动画会更慢，脚步看起来更自然
        LocomotionPlayRate = FMath::Clamp(Speed / SprintSpeed, 0.4f, 0.7f);
    }
    else
    {
        LocomotionPlayRate = 1.0f;
    }
    
    // 更新跳跃/下落状态
    const bool bCurrentlyFalling = MovementComp->IsFalling();
    bWasGrounded = bIsGrounded;
    bIsFalling = bCurrentlyFalling;
    bIsGrounded = !bCurrentlyFalling;
    
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

    // ===== 更新瞄准偏移变量（对应原蓝图 Set Roll Pitch and Yaw） =====
    // 这些变量用于 Aim Offset BlendSpace，让上半身跟随瞄准方向
    const FRotator ActorRotation = Wukong->GetActorRotation();
    const FRotator ControlRotation = Wukong->GetControlRotation();
    
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

