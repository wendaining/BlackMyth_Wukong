// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * 主菜单专用的游戏模式。负责创建并显示主菜单界面，
 * 并将输入切换为仅 UI 模式，方便玩家用鼠标操作菜单。
 */
UCLASS()
class BLACKMYTH_API AMainMenuGameMode : public AGameModeBase {
    GENERATED_BODY()

public:
    /**
     * 在 BeginPlay 时创建主菜单 UI，并设置为仅 UI 的输入模式。
     */
    virtual void BeginPlay() override;

protected:
    /** 主菜单的蓝图 Widget 类（在编辑器中设置）。 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> MainMenuWidgetClass;
    /** 运行时创建的主菜单实例。 */
    UPROPERTY()
    class UUserWidget* MainMenuWidget;
};
