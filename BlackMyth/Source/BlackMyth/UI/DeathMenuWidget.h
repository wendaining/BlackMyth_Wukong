// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathMenuWidget.generated.h"

/**
 * 死亡菜单UI Widget
 * 玩家死亡时显示，提供重生和退出游戏选项
 */
UCLASS()
class BLACKMYTH_API UDeathMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	/** 重生按钮点击事件 */
	UFUNCTION(BlueprintCallable, Category = "DeathMenu")
	void OnRespawnClicked();

	/** 退出游戏按钮点击事件 */
	UFUNCTION(BlueprintCallable, Category = "DeathMenu")
	void OnQuitGameClicked();

	/** 重生按钮（在蓝图中绑定） */
	UPROPERTY(meta = (BindWidget))
	class UButton* RespawnButton;

	/** 退出游戏按钮（在蓝图中绑定） */
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitGameButton;

	/** 重生按钮文本（可选，在蓝图中绑定） */
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* RespawnButtonText;

	/** 退出游戏按钮文本（可选，在蓝图中绑定） */
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* QuitGameButtonText;
};
