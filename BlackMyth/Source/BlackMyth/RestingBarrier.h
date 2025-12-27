// 安息术屏障 - 参考BossCombatTrigger的空气墙实现
// 使用圆柱形StaticMesh作为物理墙体，阻挡敌人进入

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RestingBarrier.generated.h"

class UNiagaraComponent;

/**
 * 安息术屏障Actor
 * 使用圆柱形空气墙，阻挡敌人进入，悟空可在内部自由移动但不能离开
 */
UCLASS()
class BLACKMYTH_API ARestingBarrier : public AActor
{
	GENERATED_BODY()
	
public:	
	ARestingBarrier();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** 初始化屏障 */
	void InitializeBarrier(float InDuration);

	/** 动态设置屏障半径（蓝图可调用） */
	UFUNCTION(BlueprintCallable, Category = "RestingBarrier")
	void SetBarrierRadius(float NewRadius);

	/** 激活/关闭屏障 */
	UFUNCTION(BlueprintCallable, Category = "RestingBarrier")
	void SetBarrierActive(bool bActive);

protected:
	/** 根组件（场景组件） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootSceneComponent;

	/** 圆柱形空气墙网格（物理阻挡敌人） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WallMesh;

	/** 地面圆形标记网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* FloorMesh;

	/** 特效组件（红色光环，可选） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* BarrierEffect;

	/** 屏障持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BarrierDuration = 8.0f;

	/** 屏障半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BarrierRadius = 300.0f;

	/** 屏障高度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BarrierHeight = 500.0f;

	/** 空气墙材质（红色半透明发光） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UMaterialInterface* WallMaterial;

	/** 地面圆形材质 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UMaterialInterface* FloorMaterial;

	/** 红色光环特效（可选） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UNiagaraSystem* BarrierEffectAsset;

	/** 生成音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SpawnSound;

	/** 消失音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DespawnSound;

private:
	/** 约束悟空不能离开圆形区域 */
	void ConstrainCasterToCircle();

	/** 清理屏障 */
	void CleanupBarrier();

	/** 更新墙体和地面的尺寸 */
	void UpdateWallScale();

	FTimerHandle CleanupTimer;

	/** 施法者（悟空）指针 */
	class AWukongCharacter* CasterCharacter;

	/** 屏障中心位置 */
	FVector BarrierCenter;

	/** 屏障是否激活 */
	bool bIsActive = false;
};
