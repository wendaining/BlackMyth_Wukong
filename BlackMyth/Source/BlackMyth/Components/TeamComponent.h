// 阵营管理组件 - 用于敌我判定

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamComponent.generated.h"

/**
 * 阵营枚举
 */
UENUM(BlueprintType)
enum class ETeam : uint8
{
	Player      UMETA(DisplayName = "Player"),       // 玩家阵营（包括分身）
	Enemy       UMETA(DisplayName = "Enemy"),        // 敌人阵营
	Neutral     UMETA(DisplayName = "Neutral"),      // 中立（不主动攻击）
	Environment UMETA(DisplayName = "Environment")   // 环境（陷阱等）
};

// 阵营变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamChanged, ETeam, OldTeam, ETeam, NewTeam);

/**
 * 阵营管理组件
 * 挂载到需要阵营判定的 Actor 上
 * 用于敌我识别、仇恨系统等
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 核心接口 ==========

	/** 获取当前阵营 */
	UFUNCTION(BlueprintPure, Category = "Team")
	ETeam GetTeam() const { return CurrentTeam; }

	/** 设置阵营 */
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(ETeam NewTeam);

	/** 检查是否与另一个阵营敌对 */
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsHostileTo(ETeam OtherTeam) const;

	/** 检查是否与另一个 Actor 敌对（通过其 TeamComponent） */
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsHostileToActor(AActor* OtherActor) const;

	/** 检查是否与另一个 Actor 是友军 */
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsAllyOf(AActor* OtherActor) const;

	/** 检查是否是中立的（对所有阵营都不主动敌对） */
	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsNeutral() const { return CurrentTeam == ETeam::Neutral; }

	// ========== 静态辅助函数 ==========

	/** 获取 Actor 的阵营（如果有 TeamComponent） */
	UFUNCTION(BlueprintPure, Category = "Team", meta = (DisplayName = "Get Actor Team"))
	static ETeam GetActorTeam(AActor* Actor);

	/** 检查两个 Actor 是否敌对 */
	UFUNCTION(BlueprintPure, Category = "Team", meta = (DisplayName = "Are Actors Hostile"))
	static bool AreActorsHostile(AActor* ActorA, AActor* ActorB);

	/** 检查两个 Actor 是否是友军 */
	UFUNCTION(BlueprintPure, Category = "Team", meta = (DisplayName = "Are Actors Allies"))
	static bool AreActorsAllies(AActor* ActorA, AActor* ActorB);

	/** 从 Actor 获取 TeamComponent */
	UFUNCTION(BlueprintPure, Category = "Team", meta = (DisplayName = "Get Team Component"))
	static UTeamComponent* GetTeamComponent(AActor* Actor);

	// ========== 委托 ==========

	/** 阵营变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Team|Events")
	FOnTeamChanged OnTeamChanged;

protected:
	/** 当前阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeam CurrentTeam = ETeam::Neutral;

	/** 阵营敌对关系表（内部使用，不暴露给反射系统） */
	TMap<ETeam, TArray<ETeam>> HostileTeams;

private:
	/** 初始化默认敌对关系 */
	void InitializeDefaultHostility();
};
