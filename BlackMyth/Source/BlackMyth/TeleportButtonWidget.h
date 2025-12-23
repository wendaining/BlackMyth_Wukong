#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeleportButtonWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 传送按钮控件
 * 在传送菜单中显示单个土地庙的传送按钮
 * 点击后可将玩家传送到指定土地庙位置
 */
UCLASS()
class BLACKMYTH_API UTeleportButtonWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 目标土地庙的唯一标识符
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
    FName TargetTempleID;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;

    // 传送按钮控件（绑定蓝图）
    UPROPERTY(meta = (BindWidget))
    UButton* TeleportButton;

    // 土地庙名称文本控件（绑定蓝图）
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TempleNameText;

private:
    /**
     * 传送按钮点击回调
     * 执行传送逻辑并关闭UI
     */
    UFUNCTION()
    void OnTeleportClicked();
};
