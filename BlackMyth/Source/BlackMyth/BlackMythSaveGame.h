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

    // 存档名称（你输入的）
    UPROPERTY()
    FString SaveName;

    // 存档时间
    UPROPERTY()
    FDateTime SaveTime;
};
