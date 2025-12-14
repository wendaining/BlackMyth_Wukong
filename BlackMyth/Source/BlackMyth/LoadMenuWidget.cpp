// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadMenuWidget.h"
#include "BlackMythSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void ULoadMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateSlotInfo(1, LoadSlot1Text);
    UpdateSlotInfo(2, LoadSlot2Text);
    UpdateSlotInfo(3, LoadSlot3Text);
}

void ULoadMenuWidget::UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text)
{
    if (!Text) {
        return;
    }

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
        UBlackMythSaveGame* SaveGame =
            Cast<UBlackMythSaveGame>(
                UGameplayStatics::LoadGameFromSlot(SlotName, 0));

        if (SaveGame) {
            Text->SetText(FText::FromString(SaveGame->SaveName));
            return;
        }
    }

    Text->SetText(FText::FromString(TEXT("空存档")));
}

void ULoadMenuWidget::OnLoadSlotClicked(int32 SlotIndex)
{
    if (SlotIndex < 1) {
        return;
    }

    const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
        return;
    }

    UBlackMythSaveGame* SaveGame =
        Cast<UBlackMythSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveGame) {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) {
        return;
    }

    // 确保游戏未暂停
    UGameplayStatics::SetGamePaused(World, false);

    // 恢复玩家位置（假设当前关卡就是存档关卡）
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (Player) {
        Player->SetActorLocation(SaveGame->PlayerLocation);
        Player->SetActorRotation(SaveGame->PlayerRotation);
    }

    if (OwnerPauseWidget)
    {
        OwnerPauseWidget->RemoveFromParent();
    }
    RemoveFromParent();

    // 取消暂停
    UGameplayStatics::SetGamePaused(World, false);

    // 恢复游戏输入
    if (APlayerController* PC = World->GetFirstPlayerController()) {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}
