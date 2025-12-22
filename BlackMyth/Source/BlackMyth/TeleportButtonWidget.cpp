// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Temple.h"
#include "WukongCharacter.h"

void UTeleportButtonWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // œ‘ æ TempleID
    if (TempleNameText)
    {
        TempleNameText->SetText(FText::FromName(TargetTempleID));
    }

    if (TeleportButton)
    {
        TeleportButton->OnClicked.AddDynamic(
            this, &UTeleportButtonWidget::OnTeleportClicked
        );
    }
}

void UTeleportButtonWidget::OnTeleportClicked()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 1. ’“ÕÊº“
    AWukongCharacter* Player =
        Cast<AWukongCharacter>(
            UGameplayStatics::GetPlayerCharacter(World, 0)
        );

    if (!Player) return;

    // 2. ’“ƒø±Í Temple
    for (TActorIterator<AInteractableActor> It(World); It; ++It)
    {
        if (It->TempleID == TargetTempleID && It->TeleportPoint)
        {
            Player->SetActorLocation(
                It->TeleportPoint->GetComponentLocation()
            );
            Player->SetActorRotation(
                It->TeleportPoint->GetComponentRotation()
            );
            break;
        }
    }
}

void UTeleportButtonWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (!TempleNameText)
    {
        return;
    }
    else {
        TempleNameText->SetText(
            FText::FromName(TargetTempleID)
        );
    }

    // === ¿∂Õº£∫TargetTempleID °˙ NameToString ===
    FString NameString = TargetTempleID.ToString();

    // === ¿∂Õº£∫StringToText ===
    FText DisplayText = FText::FromString(NameString);

    // === ¿∂Õº£∫SetText ===
    TempleNameText->SetText(DisplayText);
}