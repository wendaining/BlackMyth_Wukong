#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;

/**
 * 刷怪管理器
 * 负责在指定位置生成指定类型的怪物（蓝图）
 */
UCLASS()
class BLACKMYTH_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();

    /**
     * 刷怪函数
     * @param EnemyClass   要生成的怪物蓝图类（必须继承 AEnemyBase）
     * @param Location     生成位置
     * @param Rotation     生成朝向
     * @param EnemyLevel   怪物等级（用于初始化属性）
     */
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    AEnemyBase* SpawnEnemy(
        TSubclassOf<AEnemyBase> EnemyClass,
        const FVector& Location,
        const FRotator& Rotation,
        int32 EnemyLevel = 1
    );

    UPROPERTY()
    TArray<AEnemyBase*> SpawnedEnemies;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AEnemyBase> DefaultEnemyClass;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 DefaultEnemyLevel = 1;
protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AEnemyBase>> EnemyClasses;
};
