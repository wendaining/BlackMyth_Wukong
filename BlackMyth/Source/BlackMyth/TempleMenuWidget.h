// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TempleMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UTempleMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable)
    void OnTeleportClicked();

    UFUNCTION(BlueprintCallable)
    void OnTradeClicked();

    UFUNCTION(BlueprintCallable)
    void OnQuitClicked();

};