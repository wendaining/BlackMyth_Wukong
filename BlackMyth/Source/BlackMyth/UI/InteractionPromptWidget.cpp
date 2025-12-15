// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionPromptWidget.h"
#include "Components/TextBlock.h"

void UInteractionPromptWidget::ShowPrompt(const FText& InPromptText)
{
	if (PromptText)
	{
		PromptText->SetText(InPromptText);
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UInteractionPromptWidget::HidePrompt()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
