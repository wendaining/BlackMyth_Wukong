// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::StartGame()
{
    // 1. 关闭主菜单 UI
    RemoveFromParent();

    // 2. 恢复游戏输入，隐藏鼠标
    if (UWorld* world = GetWorld()) {
        if (APlayerController* pc = world->GetFirstPlayerController()) {
            pc->SetInputMode(FInputModeGameOnly());
            pc->bShowMouseCursor = false;
        }
    }
    // 3. 进入游戏地图
    UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap")));
}

void UMainMenuWidget::QuitGame()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    // 关闭 UI（可选）并退出游戏
    RemoveFromParent();

    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
}

void UMainMenuWidget::OpenSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;
    UClass* settings_widget_class = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_SettingsMenu.WBP_SettingsMenu_C"));

    if (settings_widget_class == nullptr) {
        return;
    }

    // 创建并显示设置界面
    if (UUserWidget* settings_ui = CreateWidget<UUserWidget>(World, settings_widget_class)) {
        settings_ui->AddToViewport();
    }
}
