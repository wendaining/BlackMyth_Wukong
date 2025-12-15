// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "../Dialogue/DialogueData.h"

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定继续按钮（如果存在）
	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnContinueButtonClicked);
	}
}

void UDialogueWidget::ShowDialogue(const FDialogueEntry& DialogueEntry, int32 CurrentIndex, int32 TotalCount)
{
	UE_LOG(LogTemp, Log, TEXT("DialogueWidget: ShowDialogue called - Speaker: %s, Text: %s"), 
		*DialogueEntry.SpeakerName.ToString(), *DialogueEntry.DialogueText.ToString());

	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(DialogueEntry.SpeakerName);
		SpeakerNameText->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Set SpeakerNameText"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueWidget: SpeakerNameText is NULL! Check BindWidget name in WBP_Dialogue"));
	}

	if (DialogueText)
	{
		DialogueText->SetText(DialogueEntry.DialogueText);
		DialogueText->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Set DialogueText"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueWidget: DialogueText is NULL! Check BindWidget name in WBP_Dialogue"));
	}

	if (ProgressText)
	{
		FText ProgressString = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentIndex + 1, TotalCount));
		ProgressText->SetText(ProgressString);
		ProgressText->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Set ProgressText to %s"), *ProgressString.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: ProgressText is NULL (optional)"));
	}

	// 强制设置整个Widget可见
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: *** VISIBILITY SET TO VISIBLE *** IsInViewport: %d"), IsInViewport());
}

void UDialogueWidget::HideDialogue()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueWidget::OnContinueButtonClicked()
{
	OnContinueClicked.Broadcast();
}
