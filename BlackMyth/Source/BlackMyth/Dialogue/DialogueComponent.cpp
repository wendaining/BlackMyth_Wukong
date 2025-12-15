// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueComponent.h"
#include "DialogueData.h"
#include "../UI/DialogueWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsPlaying = false;
	CurrentDialogueIndex = 0;
	DialogueWidgetInstance = nullptr;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// 加载对话数据
	LoadDialogueData();

	// 创建UI实例
	if (PlayerController && DialogueWidgetClass)
	{
		DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PlayerController, DialogueWidgetClass);
		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->AddToViewport(500);
			DialogueWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			UE_LOG(LogTemp, Log, TEXT("DialogueComponent: DialogueWidget created"));
		}
	}
}

void UDialogueComponent::LoadDialogueData()
{
	CachedDialogueSequence.Empty();

	if (DialogueConfig.bUseDataTable && DialogueConfig.DialogueDataTable)
	{
		// 从DataTable加载
		TArray<FDialogueEntry*> AllRows;
		DialogueConfig.DialogueDataTable->GetAllRows<FDialogueEntry>(TEXT("LoadDialogue"), AllRows);
		
		for (FDialogueEntry* Row : AllRows)
		{
			if (Row)
			{
				CachedDialogueSequence.Add(*Row);
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Loaded %d dialogues from DataTable"), CachedDialogueSequence.Num());
	}
	else
	{
		// 使用手动配置
		CachedDialogueSequence = DialogueConfig.ManualDialogueSequence;
		UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Using %d manual dialogues"), CachedDialogueSequence.Num());
	}
}

void UDialogueComponent::StartDialogue()
{
	if (bIsPlaying || CachedDialogueSequence.Num() == 0 || !PlayerController)
	{
		return;
	}

	CurrentDialogueIndex = 0;
	bIsPlaying = true;

	// 禁用玩家输入
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	// 广播事件
	OnDialogueStateChanged.Broadcast(true);

	// 播放第一句
	PlayCurrentDialogue();

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Started dialogue '%s' with %d lines"), 
		*DialogueConfig.TableName.ToString(), CachedDialogueSequence.Num());
}

void UDialogueComponent::NextDialogue()
{
	if (!bIsPlaying)
	{
		return;
	}

	// 清除自动播放计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoPlayTimerHandle);
	}

	CurrentDialogueIndex++;

	if (CurrentDialogueIndex >= CachedDialogueSequence.Num())
	{
		EndDialogue();
		return;
	}

	PlayCurrentDialogue();
}

void UDialogueComponent::EndDialogue()
{
	if (!bIsPlaying)
	{
		return;
	}

	bIsPlaying = false;

	// 清除计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoPlayTimerHandle);
	}

	// 隐藏对话UI
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->HideDialogue();
	}

	// 恢复玩家输入
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}

	// 广播事件
	OnDialogueStateChanged.Broadcast(false);

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Dialogue ended"));
}

void UDialogueComponent::PlayCurrentDialogue()
{
	if (CurrentDialogueIndex < 0 || CurrentDialogueIndex >= CachedDialogueSequence.Num())
	{
		return;
	}

	const FDialogueEntry& CurrentDialogue = CachedDialogueSequence[CurrentDialogueIndex];

	// 显示对话UI
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->ShowDialogue(
			CurrentDialogue,
			CurrentDialogueIndex,
			CachedDialogueSequence.Num()
		);
	}

	// 广播事件
	OnDialogueUpdated.Broadcast(CurrentDialogue, CurrentDialogueIndex);

	// 自动播放
	if (CurrentDialogue.DisplayDuration > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoPlayTimerHandle,
			this,
			&UDialogueComponent::AutoPlayNextDialogue,
			CurrentDialogue.DisplayDuration,
			false
		);
	}

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: [%d/%d] %s: %s"), 
		CurrentDialogueIndex + 1, 
		CachedDialogueSequence.Num(),
		*CurrentDialogue.SpeakerName.ToString(),
		*CurrentDialogue.DialogueText.ToString());
}

void UDialogueComponent::AutoPlayNextDialogue()
{
	NextDialogue();
}
