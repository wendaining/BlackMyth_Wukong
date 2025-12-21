// Boss战斗触发器 - 玩家进入区域时触发Boss战斗BGM

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "BossCombatTrigger.generated.h"

class ABossEnemy;
class UDialogueComponent;

/** Boss战斗触发器，当玩家进入时切换到Boss战斗状态*/
UCLASS(BlueprintType, Blueprintable)
class BLACKMYTH_API ABossCombatTrigger : public ATriggerVolume
{
	GENERATED_BODY()

public:
	ABossCombatTrigger();

	/** 设置结界激活状态 (视觉显现 + 物理阻挡) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Arena")
	void SetBarrierActive(bool bActive);

	/** 开始过场动画逻辑 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Cutscene")
	void StartBossCutscene(APawn* PlayerPawn);

	/** 过场动画结束后的回调 */
	UFUNCTION()
	void OnCutsceneFinished(bool bIsPlaying);

	/** 处理对话中的特殊事件 (如在 CG 中召唤哮天犬) */
	UFUNCTION()
	void HandleDialogueEvent(const FString& EventTag);

	/** 处理镜头切换请求 */
	UFUNCTION()
	void HandleCameraTargetChanged(AActor* NewTarget);

protected:
	virtual void BeginPlay() override;

	// 碰撞事件
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

public:

	/** 关联的Boss引用（可选，用于监听Boss死亡事件自动结束战斗） */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Boss")
	TObjectPtr<ABossEnemy> LinkedBoss;

	/** 是否在玩家离开区域时结束Boss战斗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool bEndCombatOnLeave = false;

	/** 是否只触发一次（玩家首次进入后禁用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool bTriggerOnce = true;

	/** 进入触发器时是否自动显示Boss血条 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	bool bShowBossHealthOnEnter = true;

	/** 离开触发器时是否自动隐藏Boss血条 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	bool bHideBossHealthOnLeave = true;

	/** 结界视觉组件 (四面墙) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> WallN;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> WallS;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> WallE;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> WallW;

	/** 对话/剧情组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDialogueComponent> DialogueComponent;

	/** 镜头切换混合时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Cutscene")
	float CameraBlendTime = 1.5f;

	/** 结界的金光材质 (GLOW_Gold) */
	UPROPERTY(EditAnywhere, Category = "Boss|Arena")
	TObjectPtr<class UMaterialInterface> BarrierMaterial;

	/** 进入区域后，延迟多久才开始 CG (给玩家一点时间走位到场中心，避免被关进门外) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Cutscene")
	float CutsceneStartDelay = 1.0f;

protected:
	/** 是否已触发过 */
	bool bHasTriggered = false;

	/** 绑定Boss死亡事件 */
	void BindBossDeathEvent();

	/** Boss死亡回调 */
	UFUNCTION()
	void OnBossDeath(AActor* Killer);
};
