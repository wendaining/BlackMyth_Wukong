// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "BlackMythPlayerController.h"

void UPauseMenuWidget::OnResumeClicked()
{
    // 1. 恢复游戏暂停状态
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // 2. 关闭暂停菜单（移除自己）
    RemoveFromParent();

    // 3. 恢复输入模式和鼠标隐藏
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UPauseMenuWidget::OnLoadClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) {
        return;
    }

    // 加载读档界面蓝图类
    UClass* LoadWidgetClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_LoadMenu.WBP_LoadMenu_C")
    );

    // 创建并显示读档界面
    if (UUserWidget* LoadUI = CreateWidget<UUserWidget>(PC, LoadWidgetClass)) {
        LoadUI->AddToViewport();
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void UPauseMenuWidget::OnSaveClicked()
{
    UWorld* World = GetWorld();
    if (!World) {
        return;
    }

    // 加载存档界面蓝图类
    UClass* SaveWidgetClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_SaveMenu.WBP_SaveMenu_C")
    );

    // 创建并显示存档界面
    if (UUserWidget* SaveUI = CreateWidget<UUserWidget>(World, SaveWidgetClass)) {
        SaveUI->AddToViewport();
    }
}

void UPauseMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}
