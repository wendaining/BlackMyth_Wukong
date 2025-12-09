// 激活TraceHitbox动画通知

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ActivateHitbox.generated.h"

/**
 * 激活 TraceHitbox 动画通知
 * 在攻击动画的起始帧添加，开启伤害检测窗口
 */
UCLASS(meta = (DisplayName = "Activate Hitbox"))
class BLACKMYTH_API UAnimNotify_ActivateHitbox : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_ActivateHitbox();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override 
	{ 
		return TEXT("Activate Hitbox"); 
	}

public:
	/** 要激活的 Hitbox 组件名称（留空则激活所有 HitboxComponent） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FName TraceHitboxComponentName;

	/** 本次攻击的基础伤害（0 = 使用组件默认值） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	float OverrideDamage = 0.0f;

	// ========== 其它攻击类型 ==========

	/** 是否为重击（影响伤害倍率） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Attack Type")
	bool bIsHeavyAttack = false;

	/** 是否为空中攻击（影响伤害倍率） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Attack Type")
	bool bIsAirAttack = false;
};
