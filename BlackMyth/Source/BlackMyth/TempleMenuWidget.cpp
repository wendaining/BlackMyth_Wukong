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
    APlayerController* PC = GetOwningPlayer();
    if (!PC) {
        return;
    }

    // ���� Teleport �˵���ͼ
    TSubclassOf<UTeleportMenuWidget> TeleportWidgetClass =
        LoadClass<UTeleportMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TeleportMenu.WBP_TeleportMenu_C")
        );

    if (!TeleportWidgetClass) {
        return;
    }

    // ���� Teleport �˵�
    UTeleportMenuWidget* TeleportMenu =
        CreateWidget<UTeleportMenuWidget>(PC, TeleportWidgetClass);

    if (!TeleportMenu) {
        return;
    }

    // �����������Ǵ� TempleMenu �򿪵�
    TeleportMenu->OwnerTempleWidget = this;

    // �����Լ�����ѡ��
    RemoveFromParent();

    // ��ʾ Teleport �˵�
    TeleportMenu->AddToViewport();
    UE_LOG(LogTemp, Warning, TEXT("TeleportMenu created"));

    // UI ����ģʽ
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnTradeClicked()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) {
        return;
    }

    // ���� Trade �˵���ͼ
    TSubclassOf<UTradeMenuWidget> TradeWidgetClass =
        LoadClass<UTradeMenuWidget>(
            nullptr,
            TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_TradeMenu.WBP_TradeMenu_C")
        );

    if (!TradeWidgetClass) {
        return;
    }

    // ���� Trade �˵�
    UTradeMenuWidget* TradeMenu =
        CreateWidget<UTradeMenuWidget>(PC, TradeWidgetClass);

    if (!TradeMenu) {
        return;
    }

    // �����������Ǵ� TempleMenu �򿪵�
    TradeMenu->OwnerTempleWidget = this;

    // �����Լ�����ѡ��
    RemoveFromParent();

    // ��ʾ Teleport �˵�
    TradeMenu->AddToViewport();

    // UI ����ģʽ
    PC->SetInputMode(FInputModeUIOnly());
    PC->bShowMouseCursor = true;
}

void UTempleMenuWidget::OnQuitClicked()
{
    // 1. �ָ���Ϸ��ͣ״̬
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // 2. �ر���ͣ�˵����Ƴ��Լ���
    RemoveFromParent();

    // 3. �ָ�����ģʽ���������
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}
