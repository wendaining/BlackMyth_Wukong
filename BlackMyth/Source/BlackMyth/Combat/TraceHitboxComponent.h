// 射线扫描 TraceHitbox 组件

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageTypes.h"
#include "TraceHitboxComponent.generated.h"

// 前向声明
class UCombatComponent;
class UHealthComponent;
class AEnemyBase;

// 命中事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTraceHitDetected, AActor*, HitActor, const FHitResult&, HitResult);

// 状态变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTraceStateChanged, bool, bIsActive);

/**
 * 射线扫描 Hitbox 组件
 * 每帧从武器起点到终点进行球形扫描，精确检测攻击命中
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UTraceHitboxComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTraceHitboxComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== 激活控制 ==========

	/** 激活扫描（开始检测） */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox")
	void ActivateTrace();

	/** 停用扫描（停止检测） */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox")
	void DeactivateTrace();

	/** 是否正在扫描 */
	UFUNCTION(BlueprintPure, Category = "TraceHitbox")
	bool IsTraceActive() const { return bIsActive; }

	/** 清除已命中列表（允许再次命中同一目标） */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox")
	void ClearHitActors();

	// ========== 配置 ==========

	/** 设置武器起点 Socket/Bone 名称 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Config")
	void SetStartSocket(FName SocketName) { StartSocketName = SocketName; }

	/** 设置武器终点 Socket/Bone 名称 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Config")
	void SetEndSocket(FName SocketName) { EndSocketName = SocketName; }

	/** 设置扫描半径 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Config")
	void SetTraceRadius(float Radius) { TraceRadius = Radius; }

	// ========== CombatComponent 对接 ==========

	/** 设置关联的战斗组件 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox")
	void SetCombatComponent(UCombatComponent* InCombatComponent);

	/** 获取关联的战斗组件 */
	UFUNCTION(BlueprintPure, Category = "TraceHitbox")
	UCombatComponent* GetCombatComponent() const;

	// ========== 攻击类型 ==========

	/** 设置是否为重击 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Attack")
	void SetHeavyAttack(bool bHeavy) { bIsHeavyAttack = bHeavy; }

	/** 设置是否为空中攻击 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Attack")
	void SetAirAttack(bool bAir) { bIsAirAttack = bAir; }

	/** 当前是否为重击 */
	UFUNCTION(BlueprintPure, Category = "TraceHitbox|Attack")
	bool IsHeavyAttack() const { return bIsHeavyAttack; }

	/** 当前是否为空中攻击 */
	UFUNCTION(BlueprintPure, Category = "TraceHitbox|Attack")
	bool IsAirAttack() const { return bIsAirAttack; }

	// ========== 伤害配置 ==========

	/** 设置基础伤害 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Damage")
	void SetBaseDamage(float Damage) { DamageInfo.BaseDamage = Damage; }

	/** 设置伤害类型 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Damage")
	void SetDamageType(EDamageType Type) { DamageInfo.DamageType = Type; }

	/** 设置攻击类型 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Damage")
	void SetAttackType(EAttackType Type) { DamageInfo.AttackType = Type; }

	/** 获取伤害信息 */
	UFUNCTION(BlueprintPure, Category = "TraceHitbox|Damage")
	FDamageInfo GetDamageInfo() const { return DamageInfo; }

	// ========== 调试 ==========

	/** 设置是否显示调试绘制 */
	UFUNCTION(BlueprintCallable, Category = "TraceHitbox|Debug")
	void SetDebugDrawEnabled(bool bEnabled) { bDebugDraw = bEnabled; }

	// ========== 委托 ==========

	/** 命中目标时触发 */
	UPROPERTY(BlueprintAssignable, Category = "TraceHitbox|Events")
	FOnTraceHitDetected OnHitDetected;

	/** 状态变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "TraceHitbox|Events")
	FOnTraceStateChanged OnStateChanged;

protected:
	// ========== 核心逻辑 ==========

	/** 执行一次扫描（每帧调用） */
	void PerformTrace();

	/** 获取 Socket/Bone 世界位置 */
	FVector GetSocketLocation(FName SocketName) const;

	/** 检查骨骼或Socket是否存在 */
	bool DoesBoneOrSocketExist(FName Name) const;

	/** 检查目标是否有效 */
	bool IsValidTarget(AActor* Target) const;

	/** 对目标应用伤害 */
	void ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult);

	/** 尝试对敌人应用伤害 */
	bool TryApplyDamageToEnemy(AEnemyBase* Enemy, float FinalDamage);

	/** 绘制调试信息 */
	void DrawDebugTrace(const FVector& Start, const FVector& End, bool bHit);

protected:
	// ========== Socket 配置 ==========

	/** 武器起点 Socket/Bone 名称（握把位置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Socket")
	FName StartSocketName = FName("weapon_r");

	/** 武器终点 Socket/Bone 名称（棍尖位置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Socket")
	FName EndSocketName = FName("weapon_B_front_r");

	/** 
	 * 如果 Socket/Bone 不存在，回退使用手部骨骼
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Socket")
	FName HandBoneName = FName("hand_r");

	/** 武器长度（用于回退计算，当 Socket 不存在时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Socket")
	float WeaponLength = 120.0f;

	// ========== 扫描配置 ==========

	/** 扫描半径（球形扫描的半径） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Trace")
	float TraceRadius = 15.0f;

	/** 扫描通道 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	/** 是否使用插值扫描（捕获帧间移动，防止快速移动漏检） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Trace")
	bool bUseInterpolation = true;

	// ========== 伤害配置 ==========

	/** 默认伤害信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Damage")
	FDamageInfo DamageInfo;

	/** 是否自动应用伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Damage")
	bool bAutoApplyDamage = true;

	// ========== 攻击类型状态 ==========

	/** 当前是否为重击 */
	UPROPERTY(BlueprintReadWrite, Category = "TraceHitbox|Attack")
	bool bIsHeavyAttack = false;

	/** 当前是否为空中攻击 */
	UPROPERTY(BlueprintReadWrite, Category = "TraceHitbox|Attack")
	bool bIsAirAttack = false;

	// ========== 调试 ==========

	/** 是否绘制调试信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Debug")
	bool bDebugDraw = true;

	/** 调试颜色 - 未命中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Debug")
	FColor DebugColorMiss = FColor::Green;

	/** 调试颜色 - 命中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Debug")
	FColor DebugColorHit = FColor::Red;

	/** 调试绘制持续时间（0 = 一帧） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TraceHitbox|Debug")
	float DebugDrawDuration = 0.0f;

private:
	/** 是否正在扫描 */
	bool bIsActive = false;

	/** 已命中的 Actor 列表（防止重复伤害） */
	UPROPERTY()
	TArray<AActor*> HitActors;

	/** 上一帧的起点位置（用于插值扫描） */
	FVector LastStartLocation;

	/** 上一帧的终点位置 */
	FVector LastEndLocation;

	/** 是否有上一帧数据 */
	bool bHasLastFrameData = false;

	/** 缓存的战斗组件 */
	UPROPERTY()
	TWeakObjectPtr<UCombatComponent> CachedCombatComponent;

	/** 缓存的骨骼网格体组件 */
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;
};
