// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable)
    void OnResumeClicked();

    UFUNCTION(BlueprintCallable)
    void OnLoadClicked();

    UFUNCTION(BlueprintCallable)
    void OnQuitClicked();
};