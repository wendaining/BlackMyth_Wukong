#include "SaveMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BlackMythSaveGame.h"

void USaveMenuWidget::OnSaveSlotClicked(int32 SlotIndex) {
    if (SlotIndex < 1) {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) {
        return;
    }

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!Player) {
        return;
    }

    // 创建或覆盖存档
    UBlackMythSaveGame* SaveGame =
        Cast<UBlackMythSaveGame>(
            UGameplayStatics::CreateSaveGameObject(
                UBlackMythSaveGame::StaticClass()
            )
        );

    if (!SaveGame) {
        return;
    }

    // === 玩家状态 ===
    SaveGame->PlayerLocation = Player->GetActorLocation();
    SaveGame->PlayerRotation = Player->GetActorRotation();

    // === 存档名（来自输入框）===
    if (SaveNameTextBox && !SaveNameTextBox->GetText().IsEmpty()) {
        SaveGame->SaveName = SaveNameTextBox->GetText().ToString();
    }
    else {
        SaveGame->SaveName = FString::Printf(TEXT("Save Slot %d"), SlotIndex);
    }

    // 记录存档时间（给 LoadMenu 显示用）
    SaveGame->SaveTime = FDateTime::Now();

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
    UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);

    RemoveFromParent();
}

void USaveMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateSlotInfo(1, SaveSlot1Text);
    UpdateSlotInfo(2, SaveSlot2Text);
    UpdateSlotInfo(3, SaveSlot3Text);
}

void USaveMenuWidget::UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text)
{
    if (!Text) return;

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UBlackMythSaveGame* SaveGame =
            Cast<UBlackMythSaveGame>(
                UGameplayStatics::LoadGameFromSlot(SlotName, 0)
            );

        if (SaveGame)
        {
            Text->SetText(FText::FromString(SaveGame->SaveName));
            return;
        }
    }

    Text->SetText(FText::FromString(TEXT("空存档")));
}
