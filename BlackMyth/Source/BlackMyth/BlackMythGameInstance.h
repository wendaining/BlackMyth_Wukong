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

    // ========== 重生点系统 ==========
    
    /** 是否有已保存的重生点 */
    UPROPERTY(BlueprintReadWrite, Category = "Respawn")
    bool bHasRespawnPoint = false;

    /** 重生点位置 */
    UPROPERTY(BlueprintReadWrite, Category = "Respawn")
    FVector RespawnLocation;

    /** 重生点旋转 */
    UPROPERTY(BlueprintReadWrite, Category = "Respawn")
    FRotator RespawnRotation;

    /** 重生点所属的Temple ID（用于存档） */
    UPROPERTY(BlueprintReadWrite, Category = "Respawn")
    FName RespawnTempleID;

    /** 设置重生点 */
    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void SetRespawnPoint(FVector Location, FRotator Rotation, FName TempleID);

    /** 获取是否有重生点 */
    UFUNCTION(BlueprintPure, Category = "Respawn")
    bool HasRespawnPoint() const { return bHasRespawnPoint; }
};
