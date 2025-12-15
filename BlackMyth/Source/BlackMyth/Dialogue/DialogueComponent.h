// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

// 当前对话更新委托（UI用来显示文本）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueUpdated, const FDialogueEntry&, CurrentDialogue, int32, CurrentIndex);

/**
 * 对话组件
 * - 挂载在NPC身上
 * - 管理对话数据和播放逻辑
 * - 控制相机切换
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 对话数据（可在蓝图中配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FDialogueTable DialogueTable;

	// 对话UI类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|UI")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	// 对话状态改变事件
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStateChanged OnDialogueStateChanged;

	// 当前对话更新事件
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueUpdated OnDialogueUpdated;

	// 开始播放对话
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue();

	// 播放下一句对话（玩家点击继续时调用）
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void NextDialogue();

	// 结束对话
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	// 当前是否正在播放对话
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialoguePlaying() const { return bIsPlaying; }

	// 获取当前对话索引
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetCurrentDialogueIndex() const { return CurrentDialogueIndex; }

	// 获取对话总数
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetTotalDialogueCount() const { return DialogueTable.DialogueSequence.Num(); }

protected:
	// 播放当前对话
	void PlayCurrentDialogue();

	// 设置对话相机
	void SetupDialogueCamera(const FDialogueEntry& DialogueEntry);

	// 恢复玩家相机
	void RestorePlayerCamera();

	// 自动播放下一句对话（用于有DisplayDuration的对话）
	void AutoPlayNextDialogue();

protected:
	// 是否正在播放
	bool bIsPlaying;

	// 当前对话索引
	int32 CurrentDialogueIndex;

	// 玩家控制器引用
	UPROPERTY()
	APlayerController* PlayerController;

	// 原始相机位置和旋转（用于恢复）
	FVector OriginalCameraLocation;
	FRotator OriginalCameraRotation;
	float OriginalCameraFOV;

	// 对话UI实例
	UPROPERTY()
	UDialogueWidget* DialogueWidgetInstance;

	// 自动播放计时器
	FTimerHandle AutoPlayTimerHandle;
};
