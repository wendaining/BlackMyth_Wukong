// BlackMythGameState - 游戏状态类，管理全局场景状态

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BlackMythGameState.generated.h"

class USceneStateComponent;

/** 游戏状态类，管理全局场景状态（探索/战斗/Boss战斗）和BGM切换*/
UCLASS()
class BLACKMYTH_API ABlackMythGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABlackMythGameState();

	/** 获取场景状态组件 */
	UFUNCTION(BlueprintPure, Category = "Scene")
	USceneStateComponent* GetSceneStateComponent() const { return SceneStateComponent; }

	/** 静态辅助函数：从任意位置获取场景状态组件 */
	UFUNCTION(BlueprintCallable, Category = "Scene", meta = (WorldContext = "WorldContextObject"))
	static USceneStateComponent* GetSceneState(const UObject* WorldContextObject);

protected:
	/** 场景状态组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneStateComponent> SceneStateComponent;
};
