// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WukongAnimInstance.generated.h"

class AWukongCharacter;

/**
 * 基于悟空角色的动画实例，负责把 C++ 层的运动/状态数据实时推送给 AnimBP，
 * 这样蓝图层就能直接驱动 BlendSpace 与状态机，而无需在蓝图里重复计算速度或坠落状态。
 */
UCLASS()
class BLACKMYTH_API UWukongAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    /**
     * 速度标量，使用 BlueprintReadOnly 暴露给动画蓝图，用于 BlendSpace 样条控制。
     * 初始为 0，防止 AnimBP 在 BeginPlay 前读取未初始化的数值。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    /**
     * 是否处于下落/腾空状态，直接映射 CharacterMovement 的 IsFalling，
     * 便于 AnimBP 切换 Jump Start/Apex/Land 等循环。
     */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling = false;

private:
    /**
     * 使用弱指针缓存拥有该 AnimInstance 的悟空角色，避免每帧 Cast 造成额外开销，
     * 同时在角色销毁时自动失效，保证指针安全。
     */
    TWeakObjectPtr<AWukongCharacter> CachedWukongCharacter;

    /**
     * 缓存 Pawn Owner，若当前缓存失效则重新获取，确保后续的移动组件查询总能拿到有效数据。
     */
    void RefreshOwningCharacter();
};

