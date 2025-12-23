#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeleportButtonWidget.h"
#include "Components/PanelWidget.h"
#include "TeleportMenuWidget.generated.h"

class UPanelWidget;

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
    UPROPERTY(meta = (BindWidget))
    class UPanelWidget* ButtonContainer;

    // 返回键
    UFUNCTION(BlueprintCallable)
    void OnBackClicked();

    UFUNCTION(BlueprintCallable)
    void BuildTeleportButtons();
};

