// 测试靶子 - 用于验证 Hitbox 系统

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetDummy.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
class UBoxComponent;

/**
 * 测试靶子
 * 简单的可被攻击目标，用于测试 Hitbox 系统
 * 带有 HealthComponent，受击时显示反馈
 */
UCLASS()
class BLACKMYTH_API ATargetDummy : public AActor
{
	GENERATED_BODY()
	
public:	
	ATargetDummy();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 重置靶子（恢复生命和外观） */
	UFUNCTION(BlueprintCallable, Category = "TargetDummy")
	void ResetDummy();

protected:
	/** 受伤回调 */
	UFUNCTION()
	void OnDamageTaken(float Damage, AActor* DamageInstigator, float RemainingHealth);

	/** 死亡回调 */
	UFUNCTION()
	void OnDeath(AActor* Killer);

	/** 更新材质颜色 */
	void UpdateMaterialColor();

protected:
	// ========== 组件 ==========

	/** 根组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** 网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** 碰撞体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	/** 生命组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	// ========== 配置 ==========

	/** 正常颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy|Appearance")
	FLinearColor NormalColor = FLinearColor(0.2f, 0.6f, 0.2f);  // 绿色

	/** 受击颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy|Appearance")
	FLinearColor HitColor = FLinearColor(1.0f, 0.3f, 0.0f);  // 橙色

	/** 死亡颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy|Appearance")
	FLinearColor DeadColor = FLinearColor(0.3f, 0.3f, 0.3f);  // 灰色

	/** 死亡后自动重生时间（0 = 不重生） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy")
	float RespawnTime = 3.0f;

	/** 是否在屏幕显示伤害数字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy|Debug")
	bool bShowDamageNumbers = true;

private:
	/** 动态材质实例 */
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	/** 受击闪烁计时器 */
	float HitFlashTimer = 0.0f;

	/** 死亡状态 */
	bool bIsDead = false;
};
