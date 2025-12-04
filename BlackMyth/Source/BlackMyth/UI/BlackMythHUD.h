// 黑神话 HUD 类 - 管理游戏 UI 的显示

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlackMythHUD.generated.h"

class UPlayerHUDWidget;

/**
 * 黑神话 HUD 类
 * 负责创建和管理游戏中的 UI 界面
 */
UCLASS()
class BLACKMYTH_API ABlackMythHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABlackMythHUD();

	/** 获取玩家 HUD Widget */
	UFUNCTION(BlueprintPure, Category = "HUD")
	UPlayerHUDWidget* GetPlayerHUDWidget() const { return PlayerHUDWidget; }

protected:
	virtual void BeginPlay() override;

	/** 玩家 HUD Widget 类（在蓝图中设置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	/** 玩家 HUD Widget 实例 */
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	UPlayerHUDWidget* PlayerHUDWidget;

private:
	/** 创建并初始化 HUD */
	void CreatePlayerHUD();
};
