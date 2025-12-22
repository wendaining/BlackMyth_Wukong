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

    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEnemy failed: EnemyClass is NULL"));
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawning Enemy at %s"), *Location.ToString());

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, Location, Rotation, SpawnParams);

    if (SpawnedEnemy)
    {
        // 抬高一点，防止卡地
        FVector SafeLocation = SpawnedEnemy->GetActorLocation();
        SafeLocation.Z += 100.f;

        SpawnedEnemy->SetActorLocation(
            SafeLocation,
            false,
            nullptr,
            ETeleportType::TeleportPhysics
        );

        SpawnedEnemy->SpawnerName = GetName();
        SpawnedEnemy->InitEnemy(Level, true);
        SpawnedEnemies.Add(SpawnedEnemy);
    }


    return SpawnedEnemy;
}