#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlertIconWidget.generated.h"

class UImage;
class UWidgetAnimation;

// 警戒图标 Widget - 显示敌人头顶的警戒标志 
UCLASS()
class BLACKMYTH_API UAlertIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 设置图标可见性
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void SetIconVisibility(bool bVisible);

	// 播放警戒动画（如果有）
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void PlayAlertAnimation();

	// 停止警戒动画
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void StopAlertAnimation();

protected:
	virtual void NativeConstruct() override;

	// 警戒图标 Image 组件（需要在蓝图中绑定，命名为 "AlertIcon"）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> AlertIcon;

	// 警戒动画（可选，在蓝图中创建）
	UPROPERTY(Transient, meta = (BindWidgetAnim, OptionalWidget = true))
	TObjectPtr<UWidgetAnimation> AlertAnimation;

	// 图标默认颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert")
	FLinearColor DefaultIconColor = FLinearColor::White;

	// 警戒时图标颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert")
	FLinearColor AlertedIconColor = FLinearColor::Red;

private:
	// 是否正在播放动画
	bool bIsAnimating = false;
};
