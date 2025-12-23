#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeleportButtonWidget.h"
#include "Components/PanelWidget.h"
#include "TeleportMenuWidget.generated.h"

class UPanelWidget;
class UCanvasPanel;
class UImage;

UCLASS()
class BLACKMYTH_API UTeleportMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

public:
    // 从TempleMenu打开
    UPROPERTY()
    UUserWidget* OwnerTempleWidget;

    // 按钮蓝图类
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
    TSubclassOf<UTeleportButtonWidget> TeleportButtonClass;

    // UI 中的容器（VerticalBox / UniformGrid）
    UPROPERTY(meta = (BindWidgetOptional))
    class UPanelWidget* ButtonContainer;

    // 地图底图（可选绑定）。如果绑定到 UMG 中的 Image，将用其 Brush ImageSize 作为布局基准
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* MapImage;

    // 使用手动坐标映射（0~1 归一化）来摆放按钮（优先于自动从世界坐标归一化）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Map")
    bool bUseManualPositions = false;

    // 手动坐标映射（归一化 [0,1]）。Key 为 TempleID，Value 为在地图上的归一化位置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Map", meta=(EditCondition="bUseManualPositions"))
    TMap<FName, FVector2D> ManualNormalizedPositions;

    // 自动布局时是否反转 Y（UI Y 向下，世界 Y 可能向上）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Map")
    bool bInvertYForAutoLayout = false;

    // 按钮在 Canvas 上的像素尺寸
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Map")
    FVector2D ButtonPixelSize = FVector2D(32.f, 32.f);

private:
    // 记录动态生成的按钮，便于刷新时只清理这些而不影响固定控件（地图/返回键）
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UUserWidget>> SpawnedTeleportButtons;

    // 返回键
    UFUNCTION(BlueprintCallable)
    void OnBackClicked();

    UFUNCTION(BlueprintCallable)
    void BuildTeleportButtons();
};

