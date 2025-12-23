#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeleportButtonWidget.h"
#include "Components/PanelWidget.h"
#include "TeleportMenuWidget.generated.h"

class UPanelWidget;
class UImage;
class UCanvasPanel;

/**
 * 传送菜单控件
 * 显示所有已发现的土地庙，并在地图上标注其位置
 * 支持自动布局和手动配置两种定位模式
 */
UCLASS()
class BLACKMYTH_API UTeleportMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

public:
    // 父级土地庙菜单控件引用
    UPROPERTY()
    UUserWidget* OwnerTempleWidget;

    // 传送按钮的蓝图类
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
    TSubclassOf<UTeleportButtonWidget> TeleportButtonClass;

    // 按钮容器控件（支持VerticalBox、UniformGrid、CanvasPanel等）
    UPROPERTY(meta = (BindWidget))
    class UPanelWidget* ButtonContainer;

    // 地图背景图片控件
    UPROPERTY(meta = (BindWidget))
    class UImage* MapImage;

    // 是否使用手动配置的位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    bool bUseManualPositions = false;

    // 手动配置的土地庙位置（归一化坐标0-1）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    TMap<FName, FVector2D> ManualNormalizedPositions;

    // 按钮像素尺寸
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    FVector2D ButtonPixelSize = FVector2D(50.f, 50.f);

    // 自动布局时是否反转Y轴
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    bool bInvertYForAutoLayout = true;

    /**
     * 返回按钮点击回调
     * 关闭传送菜单并返回土地庙主菜单
     */
    UFUNCTION(BlueprintCallable)
    void OnBackClicked();

    /**
     * 构建传送按钮
     * 遍历场景中所有土地庙并创建对应的传送按钮
     */
    UFUNCTION(BlueprintCallable)
    void BuildTeleportButtons();

private:
    // 已生成的传送按钮列表
    TArray<TWeakObjectPtr<UUserWidget>> SpawnedTeleportButtons;
};

