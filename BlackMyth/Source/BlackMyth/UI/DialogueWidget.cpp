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
	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(DialogueEntry.SpeakerName);
	}

	if (DialogueText)
	{
		DialogueText->SetText(DialogueEntry.DialogueText);
	}

	if (ProgressText)
	{
		FText ProgressString = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentIndex + 1, TotalCount));
		ProgressText->SetText(ProgressString);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UDialogueWidget::HideDialogue()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueWidget::OnContinueButtonClicked()
{
	OnContinueClicked.Broadcast();
}
