// 金币掉落物 - 敌人死亡时生成，玩家可拾取

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoldPickup.generated.h"

class USphereComponent;
class AWukongCharacter;
class UWidgetComponent;

/**
 * 金币掉落物
 * 敌人死亡时生成，玩家进入范围后自动拾取
 */
UCLASS()
class BLACKMYTH_API AGoldPickup : public AActor
{
	GENERATED_BODY()

public:
	AGoldPickup();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== 组件 ==========

	/** 根组件 - 场景组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** 金币网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** 碰撞球体（用于检测玩家接近） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionSphere;

	/** 金币价值显示Widget组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> ValueWidgetComponent;

	// ========== 掉落物配置 ==========

	/** 金币数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	int32 GoldAmount = 10;

	/** 拾取音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	USoundBase* PickupSound;

	/** 拾取特效（Niagara） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class UNiagaraSystem* PickupEffect;

	/** 是否自动拾取（玩家进入范围自动拾取） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bAutoPickup = true;

	/** 自动拾取半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float AutoPickupRadius = 10.0f;

	/** 吸附半径（开始向玩家飞去） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float AttractRadius = 300.0f;

	/** 吸附速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float AttractSpeed = 800.0f;

	/** 吸附高度偏移（金币飞向玩家腰间高度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float AttractHeightOffset = 80.0f;

	/** 掉落物存活时间（秒，0表示永久） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float LifeTime = 60.0f;

	/** 掉落弹跳高度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float DropBounceHeight = 100.0f;

	/** 旋转速度（度/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float RotationSpeed = 90.0f;

	/** 上下浮动幅度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float FloatAmplitude = 10.0f;

	/** 上下浮动频率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float FloatFrequency = 2.0f;

	// ========== UI 配置 ==========

	/** 金币价值Widget类（在蓝图中配置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ValueWidgetClass;

	/** 文本显示高度偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float ValueTextHeightOffset = 40.0f;

	/** 文本显示大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector2D ValueTextDrawSize = FVector2D(50.0f, 30.0f);

	// ========== 公共接口 ==========

	/** 被拾取（玩家按F调用，或自动拾取触发） */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void PickUp(AWukongCharacter* Player);

	/** 设置金币数量 */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetGoldAmount(int32 Amount) { GoldAmount = Amount; }

	/** 玩家进入检测范围 */
	UFUNCTION()
	void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/** 开始吸附（玩家按F键时调用） */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void StartAttract();

protected:
	/** 玩家离开检测范围 */
	UFUNCTION()
	void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** 是否已被拾取（防止重复调用） */
	bool bIsPickedUp = false;

	/** 是否正在被吸附 */
	bool bIsBeingAttracted = false;

	/** 是否等待玩家按F拾取 */
	bool bWaitingForPickup = false;

	/** 吸附目标玩家 */
	UPROPERTY()
	AWukongCharacter* AttractTarget;

	/** 当前附近的玩家（用于显示提示和响应F键） */
	UPROPERTY()
	AWukongCharacter* NearbyPlayer;

	/** 初始Z位置（用于浮动效果） */
	float InitialZ = 0.0f;

	/** 浮动计时器 */
	float FloatTimer = 0.0f;

	/** 是否已落地（用于掉落动画） */
	bool bHasLanded = false;

	/** 掉落动画计时器 */
	float DropTimer = 0.0f;

	/** 掉落动画持续时间 */
	float DropDuration = 0.5f;

	/** 掉落起始位置 */
	FVector DropStartLocation;

	/** 掉落目标位置 */
	FVector DropTargetLocation;
};
