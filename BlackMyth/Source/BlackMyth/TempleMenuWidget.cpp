// Fill out your copyright notice in the Description page of Project Settings.


#include "TempleMenuWidget.h"
#include "TeleportMenuWidget.h"
#include "TradeMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/BlackMythPlayerController.h"

void UTempleMenuWidget::OnTeleportClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) {
        return;
    }

    // 加载 Teleport 菜单蓝图
    TSubclassOf<UTeleportMenuWidget> TeleportWidgetClass =
        LoadClass<UTeleportMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TeleportMenu.WBP_TeleportMenu_C")
        );

    if (!TeleportWidgetClass) {
        return;
    }

    // 创建 Teleport 菜单
    UTeleportMenuWidget* TeleportMenu =
        CreateWidget<UTeleportMenuWidget>(PC, TeleportWidgetClass);

    if (!TeleportMenu) {
        return;
    }

    // 告诉它：你是从 TempleMenu 打开的
    TeleportMenu->OwnerTempleWidget = this;

    // 隐藏自己（可选）
    RemoveFromParent();

    // 显示 Teleport 菜单
    TeleportMenu->AddToViewport();

    // UI 输入模式
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnTradeClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) {
        return;
    }

    // 加载 Trade 菜单蓝图
    TSubclassOf<UTradeMenuWidget> TradeWidgetClass =
        LoadClass<UTradeMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TradeMenu.WBP_TradeMenu_C")
        );

    if (!TradeWidgetClass) {
        return;
    }

    // 创建 Trade 菜单
    UTradeMenuWidget* TradeMenu =
        CreateWidget<UTradeMenuWidget>(PC, TradeWidgetClass);

    if (!TradeMenu) {
        return;
    }

    // 告诉它：你是从 TempleMenu 打开的
    TradeMenu->OwnerTempleWidget = this;

    // 隐藏自己（可选）
    RemoveFromParent();

    // 显示 Teleport 菜单
    TradeMenu->AddToViewport();

    // UI 输入模式
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnQuitClicked()
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
