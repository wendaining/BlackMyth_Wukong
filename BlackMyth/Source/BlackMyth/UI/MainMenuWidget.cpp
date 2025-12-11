// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::StartGame()
{
    // 1. 关闭菜单 UI
    this->RemoveFromParent();

    // 2. 恢复输入
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }

    // 3. 进入游戏关卡
    UGameplayStatics::OpenLevel(this, FName("/Game/ThirdPerson/Maps/ThirdPersonMap"));
}

void UMainMenuWidget::QuitGame()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    // 关闭 UI（可选，但建议加）
    this->RemoveFromParent();

    // 退出游戏
    UKismetSystemLibrary::QuitGame(
        this,
        PC,
        EQuitPreference::Quit,
        true
    );
}

void UMainMenuWidget::OpenSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UClass* SettingsWidgetClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_SettingsMenu.WBP_SettingsMenu_C")
    );

    UUserWidget* SettingsUI = CreateWidget<UUserWidget>(World, SettingsWidgetClass);
    if (SettingsUI)
    {
        SettingsUI->AddToViewport();
    }
}
