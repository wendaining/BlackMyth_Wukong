// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // °´Å¥µ÷ÓÃ
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void StartGame();
};
