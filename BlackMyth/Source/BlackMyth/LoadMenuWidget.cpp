// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadMenuWidget.h"
#include "BlackMythSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "WukongCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"

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

    // 恢复玩家位置
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (Player) {
        Player->SetActorLocation(SaveGame->PlayerLocation);
        Player->SetActorRotation(SaveGame->PlayerRotation);

        // 恢复主角血量和体力
        if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(Player)) {
            if (UHealthComponent* HealthComp = Wukong->GetHealthComponent()) {
                HealthComp->SetHealth(SaveGame->PlayerHealth);
            }
            if (UStaminaComponent* StaminaComp = Wukong->GetStaminaComponent()) {
                // 体力恢复
                float StaminaDiff = SaveGame->PlayerStamina - StaminaComp->GetCurrentStamina();
                if (StaminaDiff > 0) {
                    StaminaComp->RestoreStamina(StaminaDiff);
                }
                else if (StaminaDiff < 0) {
                    StaminaComp->ConsumeStamina(-StaminaDiff);
                }
            }
        }
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
