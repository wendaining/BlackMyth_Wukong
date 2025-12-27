// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueData.generated.h"

/**
 * 单条对话数据
 * 支持从CSV导入（通过DataTable）
 * 
 * CSV格式示例:
 * ---,DialogueID,SpeakerName,DialogueText,DisplayDuration,EventTag
 * Row1,Line001,老者,你好啊年轻人,0,
 * Row2,Line002,老者,这是一段很长的对话...,5,event_dialogue_complete
 */

/** 对话镜头目标 */
UENUM(BlueprintType)
enum class EDialogueCameraTarget : uint8
{
	NoChange,   // 保持当前
	Player,     // 悟空
	Boss,       // 二郎神 (AlternativeSpeaker)
	CustomTag   // 通过 Tag 查找目标
};

USTRUCT(BlueprintType)
struct FDialogueEntry : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 对话ID（用于标识）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName DialogueID;

	// 说话人名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText SpeakerName;

	// 对话内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;

	// [New] 语音/音效
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<USoundBase> VoiceSound;

	// [New] 动画蒙太奇 (通常由说话人播放)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<UAnimMontage> DialogueMontage;

	// [New] 音量倍率 (1.0 为原始音量)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float SoundVolumeMultiplier;

	// [New] 镜头切换目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EDialogueCameraTarget CameraTarget;

	// [New] 镜头锁定目标的 Tag (当 CameraTarget 为 CustomTag 时使用)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString CameraTargetTag;

	// [New] 是否作为 2D 声音播放 (勾选后无视距离，声音一直很大)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bPlaySound2D;

	// 显示时长（秒），0表示需要玩家点击继续
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float DisplayDuration;

	// 预留：触发事件标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Advanced")
	FString EventTag;

	FDialogueEntry()
		: SoundVolumeMultiplier(1.0f)
		, CameraTarget(EDialogueCameraTarget::NoChange)
		, bPlaySound2D(true)
		, DisplayDuration(0.0f)
	{
	}
};

/**
 * 对话配置（绑定到NPC的对话设置）
 */
USTRUCT(BlueprintType)
struct FDialogueTableConfig
{
	GENERATED_BODY()

public:
	// 对话表名称（显示用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText TableName;

	// 从DataTable加载对话（推荐，支持CSV导入）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* DialogueDataTable;

	// 或者手动配置对话序列（用于简单测试）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueEntry> ManualDialogueSequence;

	// 是否使用DataTable（true=从DataTable读取，false=使用手动配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bUseDataTable;

	FDialogueTableConfig()
		: DialogueDataTable(nullptr)
		, bUseDataTable(false)
	{
	}
};
