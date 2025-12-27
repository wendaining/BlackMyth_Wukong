#include "SaveMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BlackMythSaveGame.h"
#include "WukongCharacter.h"
#include "EnemySpawner.h"
#include "EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"

void USaveMenuWidget::OnSaveSlotClicked(int32 SlotIndex)
{
    // 验证槽位索引
    if (SlotIndex < 1)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!Player)
    {
        return;
    }

    // 创建存档对象
    UBlackMythSaveGame* SaveGame = Cast<UBlackMythSaveGame>(
        UGameplayStatics::CreateSaveGameObject(
            UBlackMythSaveGame::StaticClass()));

    if (!SaveGame)
    {
        return;
    }

    // 保存玩家位置和朝向
    SaveGame->PlayerLocation = Player->GetActorLocation();
    SaveGame->PlayerRotation = Player->GetActorRotation();

    // 保存玩家属性（血量和体力）
    if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(Player))
    {
        if (UHealthComponent* HealthComp = Wukong->GetHealthComponent())
        {
            SaveGame->PlayerHealth = HealthComp->GetCurrentHealth();
            SaveGame->PlayerMaxHealth = HealthComp->GetMaxHealth();
        }
        if (UStaminaComponent* StaminaComp = Wukong->GetStaminaComponent())
        {
            SaveGame->PlayerStamina = StaminaComp->GetCurrentStamina();
            SaveGame->PlayerMaxStamina = StaminaComp->GetMaxStamina();
        }
    }

    // 保存所有敌人状态
    TArray<AActor*> FoundSpawners;
    UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), FoundSpawners);

    SaveGame->Enemies.Empty();

    // 遍历所有生成器，保存其生成的敌人数据
    for (AActor* SpawnerActor : FoundSpawners)
    {
        AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor);
        if (Spawner)
        {
            for (AEnemyBase* Enemy : Spawner->SpawnedEnemies)
            {
                if (IsValid(Enemy))
                {
                    FEnemySaveData Data;
                    Enemy->WriteEnemySaveData(Data);
                    SaveGame->Enemies.Add(Data);
                }
            }
        }
    }

    // 设置存档名称（优先使用用户输入）
    if (SaveNameTextBox && !SaveNameTextBox->GetText().IsEmpty())
    {
        SaveGame->SaveName = SaveNameTextBox->GetText().ToString();
    }
    else
    {
        SaveGame->SaveName = FString::Printf(TEXT("Save Slot %d"), SlotIndex);
    }

    // 记录存档时间戳
    SaveGame->SaveTime = FDateTime::Now();

    // 写入存档文件
    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
    UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);

    // 关闭保存菜单UI
    RemoveFromParent();
}

void USaveMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 初始化时更新所有存档槽位信息
    UpdateSlotInfo(1, SaveSlot1Text);
    UpdateSlotInfo(2, SaveSlot2Text);
    UpdateSlotInfo(3, SaveSlot3Text);
}

void USaveMenuWidget::UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text)
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
            // 显示已有存档的名称
            Text->SetText(FText::FromString(SaveGame->SaveName));
            return;
        }
    }

    // 存档不存在，显示空存档
    Text->SetText(FText::FromString(TEXT("空存档")));
}
