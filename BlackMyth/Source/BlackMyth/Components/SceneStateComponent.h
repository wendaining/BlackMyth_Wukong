// 场景状态组件 - 管理探索/战斗/Boss战斗状态切换和BGM播放

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "SceneStateComponent.generated.h"

class AEnemyBase;

// 场景状态枚举
UENUM(BlueprintType)
enum class ESceneState : uint8
{
	Exploration UMETA(DisplayName = "探索"),
	NormalCombat UMETA(DisplayName = "普通战斗"),
	BossCombat UMETA(DisplayName = "Boss战斗")
};

// 场景状态变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSceneStateChanged, ESceneState, OldState, ESceneState, NewState);

/** 场景状态组件 负责管理游戏场景状态（探索/战斗/Boss）以及对应的BGM播放*/
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API USceneStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USceneStateComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ========== 状态查询 ==========

	/** 获取当前场景状态 */
	UFUNCTION(BlueprintPure, Category = "Scene")
	ESceneState GetCurrentState() const { return CurrentState; }

	/** 是否处于战斗状态（普通战斗或Boss战斗） */
	UFUNCTION(BlueprintPure, Category = "Scene")
	bool IsInCombat() const { return CurrentState == ESceneState::NormalCombat || CurrentState == ESceneState::BossCombat; }

	/** 是否处于探索状态 */
	UFUNCTION(BlueprintPure, Category = "Scene")
	bool IsExploring() const { return CurrentState == ESceneState::Exploration; }

	/** 是否处于Boss战斗状态 */
	UFUNCTION(BlueprintPure, Category = "Scene")
	bool IsInBossCombat() const { return CurrentState == ESceneState::BossCombat; }

	// ========== 状态切换接口 ==========

	/** 切换到指定场景状态 */
	UFUNCTION(BlueprintCallable, Category = "Scene")
	void TransitionToState(ESceneState NewState);

	/** 进入普通战斗状态（由玩家攻击或敌人警戒触发） */
	UFUNCTION(BlueprintCallable, Category = "Scene")
	void OnCombatInitiated();

	/** 进入Boss战斗状态（由Boss触发区域触发） */
	UFUNCTION(BlueprintCallable, Category = "Scene")
	void OnBossCombatInitiated();

	/** 结束战斗，返回探索状态 */
	UFUNCTION(BlueprintCallable, Category = "Scene")
	void OnCombatEnded();

	// ========== 委托 ==========

	/** 场景状态变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
	FOnSceneStateChanged OnSceneStateChanged;

	// ========== 音乐配置 ==========

	/** 探索状态BGM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> ExplorationMusic;

	/** 普通战斗BGM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> CombatMusic;

	/** Boss战斗BGM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> BossMusic;

	/** 音乐淡入时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicFadeInTime = 2.0f;

	/** 音乐淡出时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicFadeOutTime = 2.0f;

	/** 音乐音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.5f;

	// ========== 战斗检测配置 ==========

	/** 战斗范围检测半径（用于检测周围是否还有存活敌人） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatCheckRadius = 500.0f;

	/** 战斗状态检测间隔（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatCheckInterval = 1.0f;

	/** 战斗结束后延迟返回探索的时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatEndDelay = 1.0f;

protected:
	// ========== 内部状态 ==========

	/** 当前场景状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene")
	ESceneState CurrentState;

	/** 当前播放的音乐组件 */
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentMusicComponent;

	/** 战斗检测计时器句柄 */
	FTimerHandle CombatCheckTimerHandle;

	/** 战斗结束延迟计时器句柄 */
	FTimerHandle CombatEndDelayTimerHandle;

	// ========== 内部函数 ==========

	/** 播放指定状态的音乐 */
	void PlayMusicForState(ESceneState State);

	/** 停止当前音乐（带淡出） */
	void StopCurrentMusic();

	/** 定时检查战斗是否结束 */
	void CheckCombatEnd();

	/** 检查范围内是否所有敌人都死亡 */
	bool AreAllEnemiesDeadInRange();

	/** 获取玩家Pawn */
	APawn* GetPlayerPawn() const;

	/** 延迟后切换到探索状态 */
	void DelayedTransitionToExploration();
};
