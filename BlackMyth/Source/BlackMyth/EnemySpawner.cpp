#include "EnemySpawner.h"
#include "EnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BlackMythSaveGame.h"
#include "UObject/ConstructorHelpers.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

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
    if (!EnemyClass) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, Location, Rotation, SpawnParams);

    if (SpawnedEnemy)
    {
        SpawnedEnemy->SpawnerName = GetName(); // 设置Spawner名称
        SpawnedEnemy->InitEnemy(Level, true);
        SpawnedEnemies.Add(SpawnedEnemy);
    }

    return SpawnedEnemy;
}