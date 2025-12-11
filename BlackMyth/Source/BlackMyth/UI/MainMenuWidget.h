// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 主菜单界面的小部件。暴露开始、退出、设置等按钮供蓝图绑定。
 */
UCLASS()
class BLACKMYTH_API UMainMenuWidget : public UUserWidget {
    GENERATED_BODY()

public:
    /** 开始游戏（绑定到开始按钮）。 */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void StartGame();

    /** 退出游戏（绑定到退出按钮）。 */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void QuitGame();

    /** 打开设置（绑定到设置按钮）。 */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void OpenSettings();
};