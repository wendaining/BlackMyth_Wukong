// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class BLACKMYTH_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    /** 开始游戏按钮 */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void StartGame();

    /** 退出游戏按钮 */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void QuitGame();

    /** 打开设置菜单（设置按钮） */
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void OpenSettings();

};