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

	// 显示时长（秒），0表示需要玩家点击继续
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float DisplayDuration;

	// 预留：触发事件标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Advanced")
	FString EventTag;

	FDialogueEntry()
		: DisplayDuration(0.0f)
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
