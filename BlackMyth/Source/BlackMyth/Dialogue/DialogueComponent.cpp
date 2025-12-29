// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueComponent.h"
#include "DialogueData.h"
#include "../UI/DialogueWidget.h"
#include "../WukongCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "../XiaoTian.h"

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

	// 禁用玩家移动和视角输入
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	// 通知玩家进入对话状态（禁用所有操作除了E键）
	if (APawn* PlayerPawn = PlayerController->GetPawn())
	{
		if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(PlayerPawn))
		{
			Wukong->SetInDialogue(true);
			Wukong->SetActiveDialogueComponent(this);
		}
	}

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

		// 通知玩家退出对话状态
		if (APawn* PlayerPawn = PlayerController->GetPawn())
		{
			if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(PlayerPawn))
			{
				Wukong->SetInDialogue(false);
				Wukong->SetActiveDialogueComponent(nullptr);
			}
		}
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

	// 广播自定义事件标签 (用于触发放狗等逻辑)
	if (!CurrentDialogue.EventTag.IsEmpty())
	{
		OnDialogueEvent.Broadcast(CurrentDialogue.EventTag);
		UE_LOG(LogTemp, Log, TEXT("Dialogue Event Triggered: %s"), *CurrentDialogue.EventTag);
	}

	// 处理镜头切换逻辑
	if (CurrentDialogue.CameraTarget != EDialogueCameraTarget::NoChange)
	{
		AActor* CameraActor = nullptr;
		if (CurrentDialogue.CameraTarget == EDialogueCameraTarget::Player)
		{
			CameraActor = UGameplayStatics::GetPlayerPawn(this, 0);
		}
		else if (CurrentDialogue.CameraTarget == EDialogueCameraTarget::Boss)
		{
			CameraActor = AlternativeSpeaker.Get() ? AlternativeSpeaker.Get() : GetOwner();
		}
		else if (CurrentDialogue.CameraTarget == EDialogueCameraTarget::CustomTag && !CurrentDialogue.CameraTargetTag.IsEmpty())
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*CurrentDialogue.CameraTargetTag), FoundActors);
			if (FoundActors.Num() > 0)
			{
				CameraActor = FoundActors[0];
			}
			else
			{
				// 特殊处理：如果 tag 是 XiaoTian 且地图里有哮天犬
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AXiaoTian::StaticClass(), FoundActors);
				if (FoundActors.Num() > 0)
				{
					CameraActor = FoundActors[0];
				}
			}
		}

		if (CameraActor)
		{
			OnCameraTargetChanged.Broadcast(CameraActor);
			UE_LOG(LogTemp, Log, TEXT("Dialogue Camera Target Changed to: %s"), *CameraActor->GetName());
		}
	}

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

	// [New] 播放语音/音效
	if (CurrentDialogue.VoiceSound)
	{
		if (CurrentDialogue.bPlaySound2D)
		{
			// 作为 2D 声音播放（无视距离，声音清晰）
			UGameplayStatics::PlaySound2D(this, CurrentDialogue.VoiceSound, CurrentDialogue.SoundVolumeMultiplier);
		}
		else
		{
			// 作为 3D 空间声音播放（受距离衰减影响）
			AActor* SoundLocationActor = AlternativeSpeaker.Get() ? AlternativeSpeaker.Get() : GetOwner();
			if (SoundLocationActor)
			{
				UGameplayStatics::PlaySoundAtLocation(this, CurrentDialogue.VoiceSound, SoundLocationActor->GetActorLocation(), CurrentDialogue.SoundVolumeMultiplier);
			}
		}
	}

	// 播放动画蒙太奇 (如果指定了备选说话人，或者拥有者是角色)
	if (CurrentDialogue.DialogueMontage)
	{
		AActor* AnimTargetActor = AlternativeSpeaker.Get() ? AlternativeSpeaker.Get() : GetOwner();
		if (ACharacter* TargetChar = Cast<ACharacter>(AnimTargetActor))
		{
			// 确保运动组件允许根运动
			if (UCharacterMovementComponent* MoveComp = TargetChar->GetCharacterMovement())
			{
				MoveComp->SetDefaultMovementMode(); // 刷新状态
			}
			
			TargetChar->PlayAnimMontage(CurrentDialogue.DialogueMontage);
		}
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
