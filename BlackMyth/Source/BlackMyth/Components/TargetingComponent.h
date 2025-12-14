// 目标锁定组件 - 类似魂系游戏的锁定系统

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

// 目标变更委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

// 目标丢失委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetLost);

/**
 * 目标锁定组件
 * 实现魂系游戏风格的目标锁定系统
 * 
 * 功能特性：
 * - 按键切换锁定/解锁
 * - 左右切换目标
 * - 自动追踪目标（摄像机平滑跟随）
 * - 目标丢失检测（距离过远、障碍物遮挡、目标死亡）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 核心接口 ==========

	/** 切换锁定状态（按下锁定键时调用） */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ToggleLockOn();

	/** 切换到下一个目标 */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SwitchTarget(bool bRight);

	/** 获取当前锁定的目标 */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	AActor* GetLockedTarget() const { return LockedTarget; }

	/** 是否正在锁定目标 */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsTargeting() const { return bIsTargeting && LockedTarget != nullptr; }

	/** 强制解除锁定 */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearTarget();

	/** 获取锁定目标的方向（用于角色朝向） */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	FVector GetDirectionToTarget() const;

	/** 获取锁定目标的距离 */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	float GetDistanceToTarget() const;

	// ========== 委托 ==========

	/** 目标变更时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Targeting|Events")
	FOnTargetChanged OnTargetChanged;

	/** 目标丢失时触发（距离过远、死亡等） */
	UPROPERTY(BlueprintAssignable, Category = "Targeting|Events")
	FOnTargetLost OnTargetLost;

protected:
	// ========== 配置参数 ==========

	/** 最大锁定距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float TargetingDistance = 1500.0f;

	/** 目标搜索半球半径（前方扇形区域） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float TargetingRadius = 500.0f;

	/** 锁定搜索角度（前方多少度范围内） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float TargetingAngle = 90.0f;

	/** 丢失目标的距离阈值（超过此距离自动解锁） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float LoseTargetDistance = 2000.0f;

	/** 摄像机转向目标的插值速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float CameraInterpSpeed = 4.0f;

	/** 软锁定混合权重（0=完全自由, 1=完全锁定） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoftLockStrength = 0.7f;

	/** 玄学角度自由范围（上下可微调的角度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float PitchFreedom = 25.0f;

	/** 水平角度自由范围（左右可微调的角度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float YawFreedom = 25.0f;

	/** 是否锁定上下视角（默认关闭，让玩家自由上下看） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	bool bLockPitch = false;

	/** 目标高度偏移（锁定点高于目标中心的偏移） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float TargetHeightOffset = 50.0f;

	/** 是否检测障碍物遮挡 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	bool bCheckLineOfSight = true;

	/** 遮挡容忍时间（被遮挡多久后丢失目标） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	float LineOfSightLostTolerance = 1.0f;

	/** 可被锁定的Actor类（留空则锁定所有带HealthComponent的） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	TArray<TSubclassOf<AActor>> TargetableClasses;

	/** 目标筛选标签（目标必须具有此标签） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Config")
	FName TargetTag = FName("Enemy");

	// ========== 运行时状态 ==========

	/** 当前锁定的目标 */
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|Runtime")
	TObjectPtr<AActor> LockedTarget;

	/** 是否正在锁定模式 */
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|Runtime")
	bool bIsTargeting = false;

	/** 视线遮挡计时器 */
	float LineOfSightLostTimer = 0.0f;

	/** 拥有者的 PlayerController */
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerController;

	// ========== 内部方法 ==========

	/** 寻找最佳目标 */
	AActor* FindBestTarget() const;

	/** 寻找所有可锁定目标 */
	TArray<AActor*> FindAllTargets() const;

	/** 检查Actor是否可被锁定 */
	bool IsTargetValid(AActor* Target) const;

	/** 检查目标是否在视线内 */
	bool HasLineOfSight(AActor* Target) const;

	/** 检查目标是否存活 */
	bool IsTargetAlive(AActor* Target) const;

	/** 更新摄像机朝向目标 */
	void UpdateCameraToTarget(float DeltaTime);

	/** 验证当前目标是否仍然有效 */
	void ValidateCurrentTarget(float DeltaTime);

	/** 计算目标的屏幕优先级（用于切换目标） */
	float CalculateTargetScore(AActor* Target, bool bPreferRight) const;

	/** 目标死亡回调 */
	UFUNCTION()
	void OnTargetDeath(AActor* DeadActor);
};
