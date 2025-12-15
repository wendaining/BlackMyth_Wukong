// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;

/**
 * 交互提示UI
 * - 显示在屏幕中央偏下方
 * - 提示玩家按E键对话
 */
UCLASS()
class BLACKMYTH_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 显示提示
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowPrompt(const FText& PromptText);

	// 隐藏提示
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HidePrompt();

protected:
	// 提示文本（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PromptText;
};
