// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BlackMythSaveGame.h"

void USaveMenuWidget::OnSaveSlotClicked(int32 SlotIndex) {
    if (SlotIndex < 1) {
        return;
    }

    UWorld* world = GetWorld();
    if (world == nullptr) {
        return;
    }

    ACharacter* player = UGameplayStatics::GetPlayerCharacter(world, 0);
    if (player == nullptr) {
        return;
    }

    // ´´½¨»ò¸²¸Ç´æµµ
    UBlackMythSaveGame* save_game =
        Cast<UBlackMythSaveGame>(UGameplayStatics::CreateSaveGameObject(
            UBlackMythSaveGame::StaticClass()));

    save_game->PlayerLocation = player->GetActorLocation();
    save_game->PlayerRotation = player->GetActorRotation();
    save_game->SaveName = FString::Printf(TEXT("Save Slot %d"), SlotIndex);

    const FString slot_name = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
    UGameplayStatics::SaveGameToSlot(save_game, slot_name, 0);

    RemoveFromParent();
}


