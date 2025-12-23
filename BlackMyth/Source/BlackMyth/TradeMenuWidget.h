#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TradeMenuWidget.generated.h"

/**
 * 交易菜单控件
 * 提供物品买卖和交易功能
 * 可以返回到土地庙主菜单
 */
UCLASS()
class BLACKMYTH_API UTradeMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 父级土地庙菜单控件引用，用于返回功能
    UPROPERTY()
    UUserWidget* OwnerTempleWidget;

    /**
     * 返回按钮点击回调
     * 关闭交易菜单并返回土地庙主菜单
     */
    UFUNCTION(BlueprintCallable)
    void OnBackClicked();
};