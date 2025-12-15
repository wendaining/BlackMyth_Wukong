// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

class UTextBlock;
class UButton;
struct FDialogueEntry;

/**
 * 对话显示UI
 * - 显示说话人名字和对话内容
 * - 传统对话框样式
 */
UCLASS()
class BLACKMYTH_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 显示对话
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowDialogue(const FDialogueEntry& DialogueEntry, int32 CurrentIndex, int32 TotalCount);

	// 隐藏对话
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void HideDialogue();

	// 继续按钮点击事件（需要在蓝图中绑定或C++中手动绑定）
	UFUNCTION()
	void OnContinueButtonClicked();

protected:
	// 说话人名字文本（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SpeakerNameText;

	// 对话内容文本（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;

	// 进度提示文本（例如：1/3）（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ProgressText;

	// 继续按钮（可选，也可以用键盘E键）
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ContinueButton;

public:
	// 继续对话的委托（当玩家点击继续时触发）
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContinueClicked);
	
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnContinueClicked OnContinueClicked;
};
