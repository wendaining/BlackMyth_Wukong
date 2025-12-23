// Fill out your copyright notice in the Description page of Project Settings.

#include "TempleMenuWidget.h"
#include "TeleportMenuWidget.h"
#include "Components/PanelWidget.h"
#include "TradeMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/BlackMythPlayerController.h"

void UTempleMenuWidget::OnTeleportClicked()
{
    // 获取玩家控制器
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    // 动态加载传送菜单蓝图类
    TSubclassOf<UTeleportMenuWidget> TeleportWidgetClass =
        LoadClass<UTeleportMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TeleportMenu.WBP_TeleportMenu_C")
        );

    if (!TeleportWidgetClass)
    {
        return;
    }

    // 创建传送菜单实例
    UTeleportMenuWidget* TeleportMenu =
        CreateWidget<UTeleportMenuWidget>(PC, TeleportWidgetClass);

    if (!TeleportMenu)
    {
        return;
    }

    // 设置父菜单引用，支持返回功能
    TeleportMenu->OwnerTempleWidget = this;

    // 隐藏当前土地庙主菜单
    RemoveFromParent();

    // 显示传送菜单
    TeleportMenu->AddToViewport();
    UE_LOG(LogTemp, Warning, TEXT("传送菜单已创建"));

    // 确保保持UI输入模式
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnTradeClicked()
{
    // 获取玩家控制器
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    // 动态加载交易菜单蓝图类
    TSubclassOf<UTradeMenuWidget> TradeWidgetClass =
        LoadClass<UTradeMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TradeMenu.WBP_TradeMenu_C")
        );

    if (!TradeWidgetClass)
    {
        return;
    }

    // 创建交易菜单实例
    UTradeMenuWidget* TradeMenu =
        CreateWidget<UTradeMenuWidget>(PC, TradeWidgetClass);

    if (!TradeMenu)
    {
        return;
    }

    // 设置父菜单引用，支持返回功能
    TradeMenu->OwnerTempleWidget = this;

    // 隐藏当前土地庙主菜单
    RemoveFromParent();

    // 显示交易菜单
    TradeMenu->AddToViewport();

    // 确保保持UI输入模式
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnQuitClicked()
{
    // 恢复游戏运行状态
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // 关闭土地庙菜单UI
    RemoveFromParent();

    // 恢复游戏输入模式，隐藏鼠标光标
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}
