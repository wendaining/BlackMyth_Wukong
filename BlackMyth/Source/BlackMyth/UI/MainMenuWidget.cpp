// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

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