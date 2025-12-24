// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "BlackMythPlayerController.h"
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
    UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/JapaneseFeudalCastle/Levels/L_Showcase.L_Showcase")));
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
    RemoveFromParent();

    // LoadGame=1 是自定义参数
    UGameplayStatics::OpenLevel(
        this,
        FName(TEXT("/Game/JapaneseFeudalCastle/Levels/L_Showcase.L_Showcase")),
        true,
        TEXT("LoadGame=1")
    );
}

