// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueData.generated.h"

/**
 * 单条对话数据
 */
USTRUCT(BlueprintType)
struct FDialogueEntry : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 对话ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName DialogueID;

	// 说话人名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText SpeakerName;

	// 对话内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;

	// 摄像机位置偏移（相对于NPC）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Camera")
	FVector CameraOffset;

	// 摄像机旋转（Pitch, Yaw, Roll）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Camera")
	FRotator CameraRotation;

	// 摄像机视野角度(FOV)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Camera")
	float CameraFOV;

	// 显示时长（秒），<=0表示需要玩家点击继续
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float DisplayDuration;

	// 预留：触发事件标签（可以用来触发特殊效果、动画等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Advanced")
	TArray<FName> EventTags;

	// 预留：额外数据（JSON格式）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Advanced")
	FString ExtraData;

	FDialogueEntry()
		: CameraOffset(FVector(150.0f, 0.0f, 50.0f))
		, CameraRotation(FRotator(-10.0f, 180.0f, 0.0f))
		, CameraFOV(60.0f)
		, DisplayDuration(0.0f)
	{
	}
};

/**
 * 对话表数据（一个NPC的完整对话序列）
 */
USTRUCT(BlueprintType)
struct FDialogueTable
{
	GENERATED_BODY()

public:
	// 对话表ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName TableID;

	// 对话表名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText TableName;

	// 对话序列（按顺序播放）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueEntry> DialogueSequence;

	// 预留：前置条件标签（用来控制对话是否可触发）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Advanced")
	TArray<FName> PrerequisiteTags;

	FDialogueTable()
	{
	}
};
