// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueComponent.h"
#include "DialogueData.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsPlaying = false;
	CurrentDialogueIndex = 0;
	OriginalCameraFOV = 90.0f;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取玩家控制器
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
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

	// 保存原始相机数据
	if (PlayerController->PlayerCameraManager)
	{
		OriginalCameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
		OriginalCameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		OriginalCameraFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
	}

	// 禁用玩家输入
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	// 广播对话开始事件
	OnDialogueStateChanged.Broadcast(true);

	// 播放第一句对话
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

	// 清除计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoPlayTimerHandle);
	}

	bIsPlaying = false;

	// 恢复玩家输入
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}

	// 恢复相机
	RestorePlayerCamera();

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

	// 广播当前对话更新事件（UI会监听这个事件来显示文本）
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
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 计算相机位置（NPC位置 + 偏移）
	FVector NPCLocation = OwnerActor->GetActorLocation();
	FVector CameraLocation = NPCLocation + DialogueEntry.CameraOffset;

	// 设置相机位置和旋转
	PlayerController->SetControlRotation(DialogueEntry.CameraRotation);
	PlayerController->PlayerCameraManager->SetManualCameraFade(0.0f, FLinearColor::Black, false);
	
	// 注意：这里只是设置了控制器的旋转，实际的相机位置需要在Tick中平滑过渡
	// 或者使用更高级的相机系统。这里简化处理。
	if (APawn* PlayerPawn = PlayerController->GetPawn())
	{
		// 临时方案：直接teleport相机（Phase 3会用更好的方式）
		PlayerPawn->SetActorLocation(CameraLocation);
		PlayerPawn->SetActorRotation(DialogueEntry.CameraRotation);
	}

	// 设置FOV
	PlayerController->PlayerCameraManager->SetFOV(DialogueEntry.CameraFOV);
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
