// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::StartGame()
{
    // ¹Ø¿¨Ãû
    UGameplayStatics::OpenLevel(this, FName("/Game/ThirdPerson/Maps/ThirdPersonMap"));
}