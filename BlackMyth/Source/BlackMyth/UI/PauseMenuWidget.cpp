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
    UWorld* World = GetWorld();
    if (!World) {
        return;
    }

    // 加载读档界面蓝图类
    UClass* LoadWidgetClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_LoadMenu.WBP_LoadMenu_C")
    );

    if (LoadWidgetClass == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load WBP_LoadMenu class."));
        return;
    }

    // 创建并显示读档界面
    if (UUserWidget* LoadUI = CreateWidget<UUserWidget>(World, LoadWidgetClass)) {
        LoadUI->AddToViewport();
    }
}


void UPauseMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}