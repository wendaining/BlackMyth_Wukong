#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;

/**
 * 敌人生成器
 * 负责在指定位置生成指定类型的敌人，并管理生成的敌人实例
 * 支持通过蓝图配置默认敌人类型和等级
 */
UCLASS()
class BLACKMYTH_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();

    /**
     * 生成敌人
     * @param EnemyClass   要生成的敌人蓝图类（必须继承自 AEnemyBase）
     * @param Location     生成位置（世界坐标）
     * @param Rotation     生成朝向
     * @param EnemyLevel   敌人等级，用于初始化属性（默认为1）
     * @return 生成的敌人实例指针，失败时返回nullptr
     */
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    AEnemyBase* SpawnEnemy(
        TSubclassOf<AEnemyBase> EnemyClass,
        const FVector& Location,
        const FRotator& Rotation,
        int32 EnemyLevel = 1
    );

    // 已生成的敌人列表，用于统一管理和存档
    UPROPERTY()
    TArray<AEnemyBase*> SpawnedEnemies;

    // 默认敌人类型，在BeginPlay时自动生成
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AEnemyBase> DefaultEnemyClass;

    // 默认敌人等级
    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 DefaultEnemyLevel = 1;

protected:
    virtual void BeginPlay() override;

    // 可选的敌人类型列表（预留扩展用）
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AEnemyBase>> EnemyClasses;
};
