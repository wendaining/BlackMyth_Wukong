#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BlackMythSaveGame.generated.h"

UCLASS()
class BLACKMYTH_API UBlackMythSaveGame : public USaveGame {
    GENERATED_BODY()

public:
    // 玩家状态
    UPROPERTY()
    FVector PlayerLocation;

    UPROPERTY()
    FRotator PlayerRotation;

    // 主角血量和体力（若无，默认为满）
    UPROPERTY()
    float PlayerHealth = 100.0f;

    UPROPERTY()
    float PlayerMaxHealth = 100.0f;

    UPROPERTY()
    float PlayerStamina = 100.0f;

    UPROPERTY()
    float PlayerMaxStamina = 100.0f;

    // 存档名称（输入）
    UPROPERTY()
    FString SaveName;

    // 存档时间
    UPROPERTY()
    FDateTime SaveTime;
};
