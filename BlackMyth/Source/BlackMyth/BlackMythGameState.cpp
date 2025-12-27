// BlackMythGameState - 游戏状态类，管理全局场景状态

#include "BlackMythGameState.h"
#include "Components/SceneStateComponent.h"

ABlackMythGameState::ABlackMythGameState()
{
	// 创建场景状态组件
	SceneStateComponent = CreateDefaultSubobject<USceneStateComponent>(TEXT("SceneStateComponent"));
}

USceneStateComponent* ABlackMythGameState::GetSceneState(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return nullptr;
	}

	// 尝试转换为 ABlackMythGameState
	if (ABlackMythGameState* BlackMythGameState = Cast<ABlackMythGameState>(GameState))
	{
		return BlackMythGameState->GetSceneStateComponent();
	}

	// 如果不是 ABlackMythGameState，尝试查找组件
	return GameState->FindComponentByClass<USceneStateComponent>();
}
