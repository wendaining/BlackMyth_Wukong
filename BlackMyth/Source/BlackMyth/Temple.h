// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "Temple.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 * 可交互Actor基类（土地庙）
 * 提供玩家交互功能，包括触发范围检测、UI显示、传送点等
 * 实现IInteractInterface接口，支持统一的交互系统
 */
UCLASS()
class BLACKMYTH_API AInteractableActor : public AActor, public IInteractInterface
{
    GENERATED_BODY()

public:
    AInteractableActor();

protected:
    virtual void BeginPlay() override;

public:
    // 土地庙静态网格体组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Mesh;

    // 交互触发范围组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    // 交互提示UI蓝图类（玩家靠近时显示）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> TempleWidgetClass;

    // 交互提示UI实例
    UPROPERTY()
    UUserWidget* InteractWidgetInstance;

    // 交互菜单UI蓝图类（玩家按交互键时显示）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> InteractMenuWidgetClass;

    // 交互菜单UI实例
    UPROPERTY()
    UUserWidget* InteractMenuInstance;

    /**
     * 玩家进入交互范围回调
     * 显示交互提示UI并设置玩家的当前可交互对象
     */
    UFUNCTION()
    void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    /**
     * 玩家离开交互范围回调
     * 隐藏交互提示UI并清除玩家的当前可交互对象
     */
    UFUNCTION()
    void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /**
     * 执行交互逻辑
     * 蓝图可调用的交互函数
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DoInteract();

    /**
     * 交互接口实现
     * 打开土地庙菜单，恢复玩家状态，暂停游戏
     * @param Interactor 发起交互的Actor，通常是玩家角色
     */
    UFUNCTION()
    void OnInteract_Implementation(AActor* Interactor);

    /**
     * 保存重生点到GameInstance
     * @param Player 玩家角色
     */
    void SaveRespawnPoint(class AWukongCharacter* Player);

    // 土地庙唯一标识符，用于传送系统
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName TempleID;

    // 传送目标点组件，玩家传送到此位置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport")
    USceneComponent* TeleportPoint;

    // 本局游戏是否已在该土地庙获得过一瓶生命药水
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Temple")
    bool bGrantedHealthPotionThisSession = false;

    // 本局游戏是否已在该土地庙获得过一瓶体力药水
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Temple")
    bool bGrantedStaminaPotionThisSession = false;
};
