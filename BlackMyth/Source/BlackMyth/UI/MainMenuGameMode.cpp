// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;

        if (MainMenuWidgetClass)
        {
            MainMenuWidget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);

            if (MainMenuWidget)
            {
                MainMenuWidget->AddToViewport();
            }
        }
    }
}
