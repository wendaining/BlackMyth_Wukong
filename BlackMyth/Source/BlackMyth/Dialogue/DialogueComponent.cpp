// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueComponent.h"
#include "DialogueData.h"
#include "../UI/DialogueWidget.h"
#include "../UI/FadeWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsPlaying = false;
	CurrentDialogueIndex = 0;
	OriginalCameraFOV = 90.0f;
	DialogueWidgetInstance = nullptr;
	FadeWidgetInstance = nullptr;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取玩家控制器
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// 创建UI实例
	if (PlayerController)
	{
		if (DialogueWidgetClass)
		{
			UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Creating DialogueWidget..."));
			DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PlayerController, DialogueWidgetClass);
			if (DialogueWidgetInstance)
			{
				DialogueWidgetInstance->AddToViewport(500); // 非常高的优先级,确保在所有UI之上
				DialogueWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); // 初始隐藏
				UE_LOG(LogTemp, Log, TEXT("DialogueComponent: DialogueWidget created and added to viewport at ZOrder 500"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("DialogueComponent: Failed to create DialogueWidget!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: DialogueWidgetClass is NULL!"));
		}

		if (FadeWidgetClass)
		{
			UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Creating FadeWidget..."));
			FadeWidgetInstance = CreateWidget<UFadeWidget>(PlayerController, FadeWidgetClass);
			if (FadeWidgetInstance)
			{
				FadeWidgetInstance->AddToViewport(200); // 最高优先级
				FadeWidgetInstance->HideBlack();
				UE_LOG(LogTemp, Log, TEXT("DialogueComponent: FadeWidget created successfully"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("DialogueComponent: Failed to create FadeWidget!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: FadeWidgetClass is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueComponent: PlayerController is NULL in BeginPlay!"));
	}
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDialogueComponent::StartDialogue()
{
	if (bIsPlaying)
	{
		return;
	}

	if (DialogueTable.DialogueSequence.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: No dialogue data configured!"));
		return;
	}

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueComponent: PlayerController is null!"));
		return;
	}

	// 重置索引
	CurrentDialogueIndex = 0;
	bIsPlaying = true;

	// 禁用玩家移动和视角控制
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Player input disabled"));

	// 广播对话开始事件
	OnDialogueStateChanged.Broadcast(true);

	// 直接播放第一句对话（不等黑屏）
	PlayCurrentDialogue();

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Started dialogue '%s'"), *DialogueTable.TableName.ToString());
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

	// 进入下一句
	CurrentDialogueIndex++;

	// 检查是否结束
	if (CurrentDialogueIndex >= DialogueTable.DialogueSequence.Num())
	{
		EndDialogue();
		return;
	}

	// 播放下一句
	PlayCurrentDialogue();
}

void UDialogueComponent::EndDialogue()
{
	if (!bIsPlaying)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Ending dialogue..."));

	// 标记对话结束
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
		UE_LOG(LogTemp, Log, TEXT("DialogueComponent: DialogueWidget hidden"));
	}

	// 立即恢复玩家输入控制（不等黑屏）
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Player input restored"));
	}

	// 广播对话结束事件
	OnDialogueStateChanged.Broadcast(false);

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Dialogue ended"));
}

void UDialogueComponent::PlayCurrentDialogue()
{
	if (CurrentDialogueIndex < 0 || CurrentDialogueIndex >= DialogueTable.DialogueSequence.Num())
	{
		return;
	}

	const FDialogueEntry& CurrentDialogue = DialogueTable.DialogueSequence[CurrentDialogueIndex];

	// 设置相机
	SetupDialogueCamera(CurrentDialogue);

	// 显示对话UI
	if (DialogueWidgetInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Showing DialogueWidget..."));
		DialogueWidgetInstance->ShowDialogue(
			CurrentDialogue,
			CurrentDialogueIndex,
			DialogueTable.DialogueSequence.Num()
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueComponent: DialogueWidgetInstance is NULL! Cannot show dialogue."));
	}

	// 广播当前对话更新事件
	OnDialogueUpdated.Broadcast(CurrentDialogue, CurrentDialogueIndex);

	// 如果有自动播放时长，设置定时器
	if (CurrentDialogue.DisplayDuration > 0.0f)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				AutoPlayTimerHandle,
				this,
				&UDialogueComponent::AutoPlayNextDialogue,
				CurrentDialogue.DisplayDuration,
				false
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Playing dialogue [%d/%d]: %s"), 
		CurrentDialogueIndex + 1, 
		DialogueTable.DialogueSequence.Num(),
		*CurrentDialogue.DialogueText.ToString());
}

void UDialogueComponent::SetupDialogueCamera(const FDialogueEntry& DialogueEntry)
{
	// 临时禁用所有相机修改 - 等UI调通后再实现相机功能
	UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Camera control disabled for testing"));
	
	// TODO: Phase 2 实现相机功能时再启用
	// 目前保持玩家原有视角，不做任何修改
}

void UDialogueComponent::RestorePlayerCamera()
{
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	// 恢复FOV
	PlayerController->PlayerCameraManager->SetFOV(OriginalCameraFOV);

	// 恢复相机位置和旋转（这里简化处理，实际应该平滑过渡）
	// Phase 3会实现更好的过渡效果
}

void UDialogueComponent::AutoPlayNextDialogue()
{
	NextDialogue();
}
