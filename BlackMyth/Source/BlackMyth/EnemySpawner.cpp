#include "EnemySpawner.h"
#include "EnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BlackMythSaveGame.h"
#include "UObject/ConstructorHelpers.h"

AEnemySpawner::AEnemySpawner()
{
    // 生成器不需要每帧更新
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 如果配置了默认敌人类型，则在游戏开始时自动生成
    if (DefaultEnemyClass)
    {
        SpawnEnemy(
            DefaultEnemyClass,
            GetActorLocation(),
            GetActorRotation(),
            DefaultEnemyLevel
        );
    }
}

AEnemyBase* AEnemySpawner::SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass, const FVector& Location, const FRotator& Rotation, int32 Level)
{
    // 检查敌人类是否有效
    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEnemy failed: EnemyClass is NULL"));
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawning Enemy at %s"), *Location.ToString());

    // 配置生成参数
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    // 如果位置有碰撞，则自动调整位置，但保证一定会生成
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 生成敌人实例
    AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, Location, Rotation, SpawnParams);

    if (SpawnedEnemy)
    {
        // 将敌人位置向上偏移，防止卡入地面
        FVector SafeLocation = SpawnedEnemy->GetActorLocation();
        SafeLocation.Z += 100.f;

        SpawnedEnemy->SetActorLocation(
            SafeLocation,
            false,
            nullptr,
            ETeleportType::TeleportPhysics
        );

        // 设置生成器名称，用于存档系统关联
        SpawnedEnemy->SpawnerName = GetName();
        // 初始化敌人属性
        SpawnedEnemy->InitEnemy(Level, true);
        // 添加到管理列表
        SpawnedEnemies.Add(SpawnedEnemy);
    }

    return SpawnedEnemy;
}