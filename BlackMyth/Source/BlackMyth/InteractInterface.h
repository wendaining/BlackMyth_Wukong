#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

// Unreal Engine自动生成的接口类，不需要修改
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * 交互接口
 * 任何可以被玩家交互的物体都应该实现此接口
 * 例如：宝箱、NPC、可拾取物品、机关等
 */
class BLACKMYTH_API IInteractInterface
{
    GENERATED_BODY()

public:
    /**
     * 当玩家与对象交互时调用
     * @param Interactor 发起交互的Actor，通常是玩家角色
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteract(AActor* Interactor);
};