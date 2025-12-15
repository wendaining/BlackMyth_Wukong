// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FadeWidget.generated.h"

class UImage;

/**
 * 黑屏过渡UI
 * - 对话开始/结束时的淡入淡出效果
 */
UCLASS()
class BLACKMYTH_API UFadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 淡入（从透明到黑色）
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void FadeIn(float Duration = 1.0f);

	// 淡出（从黑色到透明）
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void FadeOut(float Duration = 1.0f);

	// 立即显示黑屏
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void ShowBlack();

	// 立即隐藏黑屏
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void HideBlack();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// 黑色遮罩图片（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	UImage* BlackImage;

	// 当前淡入淡出状态
	bool bIsFading;
	bool bFadingIn; // true=淡入, false=淡出
	float FadeDuration;
	float CurrentFadeTime;
};
