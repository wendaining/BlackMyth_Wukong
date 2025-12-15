// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WukongAnimInstance.generated.h"

class AWukongCharacter;
class UCharacterMovementComponent;

/** 
 * 移动状态枚举 - 将状态机逻辑移至 C++
 * 在 AnimGraph 中可以直接使用 "Blend Poses by Enum" 节点
 */
UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Walk        UMETA(DisplayName = "Walk"),
	Run         UMETA(DisplayName = "Run")
};

/**
 * 移动方向枚举 - 用于选择不同方向的动画
 * 前进、后退、左移、右移
 */
UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forward     UMETA(DisplayName = "Forward"),      // 前进 (-45 到 45 度)
	Backward    UMETA(DisplayName = "Backward"),     // 后退 (135 到 -135 度)
	Left        UMETA(DisplayName = "Left"),         // 左移 (-135 到 -45 度)
	Right       UMETA(DisplayName = "Right")         // 右移 (45 到 135 度)
};

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

    /** 是否正在走路（移动但不冲刺）- 用于区分走路和跑步动画 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsWalking = false;

    /** 是否正在冲刺 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    // ========== 方向性移动变量（用于八方向动画）==========

    /** 当前移动方向枚举（前/后/左/右） */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    EMovementDirection MovementDirection = EMovementDirection::Forward;

    /** 是否正在向前移动 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    bool bIsMovingForward = false;

    /** 是否正在向后移动（后退） */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    bool bIsMovingBackward = false;

    /** 是否正在向左移动 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    bool bIsMovingLeft = false;

    /** 是否正在向右移动 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    bool bIsMovingRight = false;

    /** 前后移动量 (-1 到 1)，负值为后退 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    float ForwardAxis = 0.0f;

    /** 左右移动量 (-1 到 1)，负值为左移 */
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Direction")
    float RightAxis = 0.0f;

    /** 
     * 动画播放速率缩放，用于区分走路和跑步
     * 走路时 ~0.5，跑步时 ~1.0
     * 可在 BlendSpace 中用此值调整动画播放速率
     */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LocomotionPlayRate = 1.0f;

    /** 当前移动状态 (C++ 计算，AnimGraph 直接使用) */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    ELocomotionState LocomotionState = ELocomotionState::Idle;

    /** 上一帧的移动状态，用于检测状态转换 */
    ELocomotionState PreviousLocomotionState = ELocomotionState::Idle;

    /** 跑步急停转走路的过渡蒙太奇 */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Transitions")
    UAnimMontage* RunToWalkMontage;

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

    // ========== 动态动画引用 (从 Character 获取) ==========
    
    /** 待机动画 */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dynamic")
    TObjectPtr<UAnimSequence> IdleAnimation;

    /** 当前行走动画（根据方向自动选择） */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dynamic")
    TObjectPtr<UAnimSequence> WalkAnimation;

	/** 当前冲刺动画（根据方向自动选择） */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Dynamic")
	TObjectPtr<UAnimSequence> SprintAnimation;

    /** 当前移动动画（根据状态和方向自动选择，Walk或Sprint） */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dynamic")
    TObjectPtr<UAnimSequence> CurrentLocomotionAnimation;

    // ========== 方向性动画引用（从 Character 获取）==========

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> WalkForwardAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> WalkBackwardAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> WalkLeftAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> WalkRightAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> SprintForwardAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> SprintBackwardAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> SprintLeftAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Directional")
    TObjectPtr<UAnimSequence> SprintRightAnimation;

	// ========== 原动画蓝图中的瞄准/旋转变量 ==========    /** Yaw 旋转角度（用于 Aim Offset） */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimYaw = 0.0f;

    /** Pitch 旋转角度（用于 Aim Offset） */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimPitch = 0.0f;

    /** Roll 旋转角度 */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimRoll = 0.0f;

    /** Yaw 变化量（用于转向动画） */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float YawDelta = 0.0f;

    /** 是否正在加速（用于判断是否有移动输入） */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;

    /** 是否播放全身动画（FullBody Montage 播放时为 true） */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsFullBody = false;

    // ========== Paragon AnimBP 兼容变量（别名） ==========
    // 这些变量与上面的变量同步，只是命名风格不同，用于兼容原 Paragon 动画蓝图
    
    /** Paragon 兼容：isAccelerating */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    bool isAccelerating = false;

    /** Paragon 兼容：isFullbody */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    bool isFullbody = false;

    /** Paragon 兼容：Pitch（用于 Aim Offset） */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    float Pitch = 0.0f;

    /** Paragon 兼容：Yaw（用于 Aim Offset） */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    float Yaw = 0.0f;

    /** Paragon 兼容：Roll */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    float Roll = 0.0f;

    /** Paragon 兼容：角色引用（供蓝图 Cast 使用） */
    UPROPERTY(BlueprintReadOnly, Category = "Paragon Compat")
    TObjectPtr<ACharacter> Character = nullptr;

private:
    /** 缓存悟空角色引用 */
    TWeakObjectPtr<AWukongCharacter> CachedWukongCharacter;
    
    /** 缓存移动组件引用 */
    TWeakObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

    /** 上一帧的 Yaw，用于计算 YawDelta */
    float PreviousYaw = 0.0f;

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

