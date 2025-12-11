// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 在 BeginPlay 中创建主菜单并设置输入为仅 UI。
    if (UWorld* world = GetWorld()) {
        APlayerController* pc = world->GetFirstPlayerController();
        if (pc != nullptr) {
            pc->SetInputMode(FInputModeUIOnly());
            pc->bShowMouseCursor = true;

            if (MainMenuWidgetClass != nullptr) {
                MainMenuWidget = CreateWidget<UUserWidget>(pc, MainMenuWidgetClass);
                if (MainMenuWidget != nullptr) {
                    MainMenuWidget->AddToViewport();
                }
            }
        }
    }
}
