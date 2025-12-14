// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BlackMythSaveGame.h"

void ULoadMenuWidget::OnLoadSlotClicked(int32 SlotIndex) {
    if (SlotIndex < 1) {
        return;
    }

    const FString slot_name = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(slot_name, 0)) {
        return;
    }

    UBlackMythSaveGame* save_game =
        Cast<UBlackMythSaveGame>(UGameplayStatics::LoadGameFromSlot(slot_name, 0));

    if (save_game == nullptr) {
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

    player->SetActorLocation(save_game->PlayerLocation);
    player->SetActorRotation(save_game->PlayerRotation);

    RemoveFromParent();
}


