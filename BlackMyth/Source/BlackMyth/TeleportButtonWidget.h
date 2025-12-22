// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeleportButtonWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class BLACKMYTH_API UTeleportButtonWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 按钮要传送到的土地庙 ID
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
    FName TargetTempleID;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;

    /** 绑定蓝图里的 Button */
    UPROPERTY(meta = (BindWidget))
    UButton* TeleportButton;

    /** 绑定蓝图里的 TextBlock */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TempleNameText;

private:
    UFUNCTION()
    void OnTeleportClicked();
};
