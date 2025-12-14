// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BlackMythSaveGame.generated.h"

/**
 * 存档数据类：保存数据。
 */
UCLASS()
class BLACKMYTH_API UBlackMythSaveGame : public USaveGame {
    GENERATED_BODY()

public:
    /** 玩家位置 */
    UPROPERTY(VisibleAnywhere, Category = "Save")
    FVector PlayerLocation;

    /** 玩家朝向 */
    UPROPERTY(VisibleAnywhere, Category = "Save")
    FRotator PlayerRotation;

    /** 存档名称 */
    UPROPERTY(VisibleAnywhere, Category = "Save")
    FString SaveName;
};
