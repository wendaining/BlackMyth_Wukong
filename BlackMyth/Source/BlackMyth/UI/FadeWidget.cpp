// Fill out your copyright notice in the Description page of Project Settings.

#include "FadeWidget.h"
#include "Components/Image.h"

void UFadeWidget::FadeIn(float Duration)
{
	if (!BlackImage)
	{
		return;
	}

	bIsFading = true;
	bFadingIn = true;
	FadeDuration = Duration;
	CurrentFadeTime = 0.0f;

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UFadeWidget::FadeOut(float Duration)
{
	if (!BlackImage)
	{
		return;
	}

	bIsFading = true;
	bFadingIn = false;
	FadeDuration = Duration;
	CurrentFadeTime = 0.0f;

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UFadeWidget::ShowBlack()
{
	if (BlackImage)
	{
		BlackImage->SetOpacity(1.0f);
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	bIsFading = false;
}

void UFadeWidget::HideBlack()
{
	if (BlackImage)
	{
		BlackImage->SetOpacity(0.0f);
		SetVisibility(ESlateVisibility::Collapsed);
	}
	bIsFading = false;
}

void UFadeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFading || !BlackImage || FadeDuration <= 0.0f)
	{
		return;
	}

	CurrentFadeTime += InDeltaTime;
	float Alpha = FMath::Clamp(CurrentFadeTime / FadeDuration, 0.0f, 1.0f);

	if (bFadingIn)
	{
		// 淡入：从0到1
		BlackImage->SetOpacity(Alpha);
	}
	else
	{
		// 淡出：从1到0
		BlackImage->SetOpacity(1.0f - Alpha);
	}

	// 完成
	if (Alpha >= 1.0f)
	{
		bIsFading = false;
		
		// 淡出完成后隐藏
		if (!bFadingIn)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
