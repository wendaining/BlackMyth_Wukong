// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../WukongCharacter.h"
#include "../BlackMythGameInstance.h"

void UDeathMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 绑定按钮点击事件
    if (RespawnButton)
    {
        RespawnButton->OnClicked.AddDynamic(this, &UDeathMenuWidget::OnRespawnClicked);
    }

    if (QuitGameButton)
    {
        QuitGameButton->OnClicked.AddDynamic(this, &UDeathMenuWidget::OnQuitGameClicked);
    }

    // 设置按钮文本（如果绑定了文本控件）
    if (RespawnButtonText)
    {
        RespawnButtonText->SetText(FText::FromString(TEXT("重生")));
    }

    if (QuitGameButtonText)
    {
        QuitGameButtonText->SetText(FText::FromString(TEXT("退出游戏")));
    }
}

void UDeathMenuWidget::OnRespawnClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[DeathMenu] Respawn button clicked"));

    // 获取玩家控制器
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[DeathMenu] Failed to get PlayerController"));
        return;
    }

    // 获取玩家角色
    AWukongCharacter* WukongChar = Cast<AWukongCharacter>(PC->GetPawn());
    if (!WukongChar)
    {
        UE_LOG(LogTemp, Error, TEXT("[DeathMenu] Failed to get WukongCharacter"));
        return;
    }

    // 先调用角色的重生函数
    WukongChar->Respawn();

    // 取消游戏暂停
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // 恢复游戏输入模式
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;

    // 关闭菜单
    RemoveFromParent();

    UE_LOG(LogTemp, Log, TEXT("[DeathMenu] Respawn completed, input restored"));
}

void UDeathMenuWidget::OnQuitGameClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[DeathMenu] Quit game button clicked"));

    // 获取玩家控制器
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        // 退出游戏
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, true);
    }
}
