#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BlackMythGameInstance.generated.h"

UCLASS()
class BLACKMYTH_API UBlackMythGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // 是否从读档进入
    UPROPERTY()
    bool bLoadGameRequested = false;

    // 选择的存档槽
    UPROPERTY()
    int32 SelectedLoadSlot = 0;
};
