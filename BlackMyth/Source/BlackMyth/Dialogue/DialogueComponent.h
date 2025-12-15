// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueData.h"
#include "DialogueComponent.generated.h"

class APlayerController;
class UDialogueWidget;

// 对话状态改变委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStateChanged, bool, bIsPlaying);

// 当前对话更新委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueUpdated, const FDialogueEntry&, CurrentDialogue, int32, CurrentIndex);

/**
 * 对话组件（简化版）
 * - 挂载在NPC身上
 * - 支持DataTable(CSV)导入对话数据
 * - 保持玩家原视角，不做相机切换
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 对话配置 ==========
	
	// 对话配置（支持DataTable或手动配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FDialogueTableConfig DialogueConfig;

	// 对话UI类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|UI")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	// ========== 事件 ==========
	
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStateChanged OnDialogueStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueUpdated OnDialogueUpdated;

	// ========== 公开方法 ==========
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void NextDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialoguePlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetCurrentDialogueIndex() const { return CurrentDialogueIndex; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetTotalDialogueCount() const { return CachedDialogueSequence.Num(); }

protected:
	// 加载对话数据（从DataTable或手动配置）
	void LoadDialogueData();
	
	// 播放当前对话
	void PlayCurrentDialogue();

	// 自动播放下一句
	void AutoPlayNextDialogue();

protected:
	bool bIsPlaying;
	int32 CurrentDialogueIndex;

	// 缓存的对话序列（从DataTable或手动配置加载）
	UPROPERTY()
	TArray<FDialogueEntry> CachedDialogueSequence;

	UPROPERTY()
	APlayerController* PlayerController;

	// UI实例
	UPROPERTY()
	UDialogueWidget* DialogueWidgetInstance;

	FTimerHandle AutoPlayTimerHandle;
};
