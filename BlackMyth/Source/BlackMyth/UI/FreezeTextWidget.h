// 定身术"定"字 UI Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FreezeTextWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 定身术"定"字显示 Widget
 * 在敌人被定身时显示在头顶的发光文字
 */
UCLASS()
class BLACKMYTH_API UFreezeTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** "定"字文本组件 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FreezeText;

	/** 发光背景图片（可选） */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GlowImage;

public:
	/** 设置文字颜色 */
	UFUNCTION(BlueprintCallable, Category = "Freeze UI")
	void SetTextColor(FLinearColor Color);

	/** 播放出现动画（可在蓝图中实现） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Freeze UI")
	void PlayAppearAnimation();

	/** 播放消失动画（可在蓝图中实现） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Freeze UI")
	void PlayDisappearAnimation();
};
