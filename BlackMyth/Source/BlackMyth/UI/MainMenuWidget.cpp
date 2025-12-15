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
    // 即将使用的地图
    // /Game/JapaneseFeudalCastle/Levels/L_Showcase.L_Showcase'
}

void UMainMenuWidget::QuitGame()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    // 关闭 UI（可选）并退出游戏
    RemoveFromParent();

    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
}

void UMainMenuWidget::LoadGame()
{
    UWorld* World = GetWorld();
    if (!World) return;
    UClass* load_widget_class = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_LoadMenu.WBP_LoadMenu_C"));

    if (load_widget_class == nullptr) {
        return;
    }

    // 创建并显示设置界面
    if (UUserWidget* load_ui = CreateWidget<UUserWidget>(World, load_widget_class)) {
        load_ui->AddToViewport();
    }
}
