// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> MainMenuWidgetClass;

    UPROPERTY()
    class UUserWidget* MainMenuWidget;
};
