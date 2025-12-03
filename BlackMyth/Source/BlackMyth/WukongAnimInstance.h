// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WukongAnimInstance.generated.h"

class AWukongCharacter;
class UCharacterMovementComponent;

/**
 * 悟空角色的动画实例类
 * 
 * 设计理念：所有动画逻辑都在 C++ 层完成，Animation Blueprint 只需要：
 * 1. 读取这里暴露的变量
 * 2. 用这些变量驱动 BlendSpace 和状态机
 * 
 * 这样可以最大程度减少蓝图逻辑，同时保持动画系统的灵活性。
 */
UCLASS()
class BLACKMYTH_API UWukongAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ========== 动画事件回调 ==========
    // 这些函数可以被 Animation Montage 中的 Notify 调用
    
    /** 攻击命中检测开始 - 在 Montage 中添加 AnimNotify 调用此函数 */
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void AnimNotify_AttackHitStart();

    /** 攻击命中检测结束 */
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void AnimNotify_AttackHitEnd();

    /** 可以接受 Combo 输入的窗口开始 */
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void AnimNotify_ComboWindowStart();

    /** Combo 窗口结束 */
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void AnimNotify_ComboWindowEnd();

protected:
    // ========== 移动状态变量（每帧自动更新）==========
    
    /** 水平移动速度标量，用于 BlendSpace */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    /** 移动方向角度（相对于角色朝向），范围 -180 到 180 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    /** 是否正在移动 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    /** 是否正在冲刺 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    /** 
     * 动画播放速率缩放，用于区分走路和跑步
     * 走路时 ~0.5，跑步时 ~1.0
     * 可在 BlendSpace 中用此值调整动画播放速率
     */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LocomotionPlayRate = 1.0f;

    // ========== 跳跃/下落状态 ==========

    /** 是否处于下落/腾空状态 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling = false;

    /** 是否刚刚起跳（用于播放 Jump Start） */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bJustJumped = false;

    /** 垂直速度，正值为上升，负值为下落 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float VerticalVelocity = 0.0f;

    // ========== 战斗状态变量 ==========

    /** 是否正在攻击（播放攻击 Montage 中） */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking = false;

    /** 当前连击索引 (0, 1, 2...) */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 ComboIndex = 0;

    /** 是否正在闪避 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsDodging = false;

    /** 是否处于硬直状态 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsHitStunned = false;

    /** 是否死亡 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsDead = false;

    // ========== 用于混合的附加变量 ==========

    /** 是否接地（与 bIsFalling 相反，但更直观） */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsGrounded = true;

    /** 是否可以过渡到下一个状态（用于状态机过渡条件） */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCanTransition = true;

private:
    /** 缓存悟空角色引用 */
    TWeakObjectPtr<AWukongCharacter> CachedWukongCharacter;
    
    /** 缓存移动组件引用 */
    TWeakObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

    /** 上一帧是否在地面 */
    bool bWasGrounded = true;

    /** 刷新角色引用缓存 */
    void RefreshOwningCharacter();

    /** 更新移动相关变量 */
    void UpdateMovementVariables();

    /** 更新战斗相关变量 */
    void UpdateCombatVariables();

    /** 计算移动方向角度 */
    float CalculateDirection(const FVector& Velocity, const FRotator& BaseRotation) const;
};

