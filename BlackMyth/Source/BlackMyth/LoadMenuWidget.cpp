#include "LoadMenuWidget.h"
#include "BlackMythSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "WukongCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"
#include "EnemyBase.h"
#include "EnemySpawner.h"

void ULoadMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 初始化时更新所有存档槽位信息
    UpdateSlotInfo(1, LoadSlot1Text);
    UpdateSlotInfo(2, LoadSlot2Text);
    UpdateSlotInfo(3, LoadSlot3Text);
}

void ULoadMenuWidget::UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text)
{
    if (!Text)
    {
        return;
    }

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    // 检查存档是否存在
    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UBlackMythSaveGame* SaveGame = Cast<UBlackMythSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SlotName, 0));

        if (SaveGame)
        {
            // 显示存档名称
            Text->SetText(FText::FromString(SaveGame->SaveName));
            return;
        }
    }

    // 存档不存在或加载失败，显示空存档
    Text->SetText(FText::FromString(TEXT("空存档")));
}

void ULoadMenuWidget::OnLoadSlotClicked(int32 SlotIndex)
{
    // 验证槽位索引
    if (SlotIndex < 1)
    {
        return;
    }

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    // 检查存档是否存在
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return;
    }

    // 加载存档数据
    UBlackMythSaveGame* SaveGame = Cast<UBlackMythSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveGame)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 确保游戏处于运行状态
    UGameplayStatics::SetGamePaused(World, false);

    // 恢复玩家状态
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (Player)
    {
        // 恢复玩家位置和朝向
        Player->SetActorLocation(SaveGame->PlayerLocation);
        Player->SetActorRotation(SaveGame->PlayerRotation);

        // 恢复玩家属性（血量和体力）
        if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(Player))
        {
            if (UHealthComponent* HealthComp = Wukong->GetHealthComponent())
            {
                HealthComp->SetHealth(SaveGame->PlayerHealth);
            }
            if (UStaminaComponent* StaminaComp = Wukong->GetStaminaComponent())
            {
                // 通过计算差值来精确恢复体力值
                float StaminaDiff = SaveGame->PlayerStamina - StaminaComp->GetCurrentStamina();
                if (StaminaDiff > 0)
                {
                    StaminaComp->RestoreStamina(StaminaDiff);
                }
                else if (StaminaDiff < 0)
                {
                    StaminaComp->ConsumeStamina(-StaminaDiff);
                }
            }
        }
    }

    // 恢复敌人状态
    TArray<AActor*> FoundSpawners;
    UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), FoundSpawners);

    // 清理当前场景中所有已生成的敌人
    for (AActor* SpawnerActor : FoundSpawners)
    {
        AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor);
        if (Spawner)
        {
            for (AEnemyBase* Enemy : Spawner->SpawnedEnemies)
            {
                if (IsValid(Enemy))
                {
                    Enemy->Destroy();
                }
            }
            Spawner->SpawnedEnemies.Empty();
        }
    }

    // 根据存档数据重新生成敌人
    for (const FEnemySaveData& Data : SaveGame->Enemies)
    {
        // 验证敌人数据有效性
        if (!Data.EnemyClass || Data.SpawnerName.IsEmpty())
        {
            continue;
        }

        // 查找对应的生成器
        AEnemySpawner* TargetSpawner = nullptr;
        for (AActor* SpawnerActor : FoundSpawners)
        {
            if (SpawnerActor->GetName() == Data.SpawnerName)
            {
                TargetSpawner = Cast<AEnemySpawner>(SpawnerActor);
                break;
            }
        }

        // 如果找不到匹配的生成器，使用第一个可用的生成器
        if (!TargetSpawner && FoundSpawners.Num() > 0)
        {
            TargetSpawner = Cast<AEnemySpawner>(FoundSpawners[0]);
        }

        // 生成敌人并恢复其状态
        if (TargetSpawner)
        {
            AEnemyBase* NewEnemy = TargetSpawner->SpawnEnemy(Data.EnemyClass, Data.Location, Data.Rotation, Data.Level);
            if (NewEnemy)
            {
                NewEnemy->LoadEnemySaveData(Data);
            }
        }
    }

    // 关闭加载菜单UI
    if (OwnerPauseWidget)
    {
        OwnerPauseWidget->RemoveFromParent();
    }
    RemoveFromParent();

    // 恢复游戏运行状态
    UGameplayStatics::SetGamePaused(World, false);

    // 恢复游戏输入模式，隐藏鼠标光标
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}
