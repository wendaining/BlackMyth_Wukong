// Fill out your copyright notice in the Description page of Project Settings.

#include "WukongAnimInstance.h"
#include "WukongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWukongAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    RefreshOwningCharacter();
}

void UWukongAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedWukongCharacter.IsValid())
    {
        RefreshOwningCharacter();
        if (!CachedWukongCharacter.IsValid())
        {
            Speed = 0.0f;
            bIsFalling = false;
            return;
        }
    }

    const AWukongCharacter* Wukong = CachedWukongCharacter.Get();
    const UCharacterMovementComponent* MovementComponent = Wukong ? Wukong->GetCharacterMovement() : nullptr;
    if (!MovementComponent)
    {
        Speed = 0.0f;
        bIsFalling = false;
        return;
    }

    // 仅取水平方向速度，避免角色上跳时的 Z 轴速度导致 BlendSpace 抖动
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    Speed = HorizontalVelocity.Size();

    // 调用移动组件的 IsFalling 来判断角色是否腾空
    bIsFalling = MovementComponent->IsFalling();
}

void UWukongAnimInstance::RefreshOwningCharacter()
{
    APawn* OwningPawn = TryGetPawnOwner();
    CachedWukongCharacter = Cast<AWukongCharacter>(OwningPawn);
}

