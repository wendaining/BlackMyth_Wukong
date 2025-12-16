#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyDodgeComponent.generated.h"

class UAnimMontage;
class AEnemyBase;

// 敌人闪避组件
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UEnemyDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyDodgeComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 尝试触发闪避 */
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool TryDodge(const FVector& ThreatDirection);

	/** 判断当前是否正在闪避 */
	UFUNCTION(BlueprintPure, Category = "Dodge")
	bool IsInDodge() const { return bIsInDodge; }

	/** 闪避动画结束回调 */
	UFUNCTION()
	void OnDodgeEnd();

protected:
	/** 闪避动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** 闪避触发概率（0-1，0表示不闪避，1表示必闪） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeChance = 0.3f;

	/** 闪避冷却时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeCooldown = 3.0f;

	/** 闪避距离（单位：cm） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeDistance = 400.0f;

	/** 闪避无敌帧时长（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float InvincibilityDuration = 0.5f;

	/** 闪避音效 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<USoundBase> DodgeSound;

private:
	/** 上次闪避的时间 */
	float LastDodgeTime;

	/** 是否正在闪避 */
	bool bIsInDodge;

	/** 缓存的敌人引用 */
	UPROPERTY()
	TObjectPtr<AEnemyBase> OwnerEnemy;

	/** 无敌帧计时器 */
	FTimerHandle InvincibilityTimer;

	/** 开启无敌帧 */
	void EnableInvincibility();

	/** 关闭无敌帧 */
	void DisableInvincibility();
};
