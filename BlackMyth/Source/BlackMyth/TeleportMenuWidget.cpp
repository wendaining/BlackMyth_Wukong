// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportMenuWidget.h"
#include "Temple.h"
#include "EngineUtils.h"

void UTeleportMenuWidget::BuildTeleportButtons()
{
    if (!ButtonContainer)
    {
        UE_LOG(LogTemp, Error, TEXT("ButtonContainer is NULL"));
        return;
    }

    if (!TeleportButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TeleportButtonClass is NULL"));
        return;
    }

    ButtonContainer->ClearChildren();

    int32 Count = 0;

    for (TActorIterator<AInteractableActor> It(GetWorld()); It; ++It)
    {
        UTeleportButtonWidget* Button =
            CreateWidget<UTeleportButtonWidget>(
                GetWorld(), TeleportButtonClass
            );

        Button->TargetTempleID = It->TempleID;
        ButtonContainer->AddChild(Button);

        Count++;
        UE_LOG(LogTemp, Warning, TEXT("Found InteractableActor: %s"), *It->GetName());
    }
    UE_LOG(LogTemp, Warning, TEXT("Total InteractableActors: %d"), Count);
}

void UTeleportMenuWidget::OnBackClicked() {
    RemoveFromParent();
    if (OwnerTempleWidget) {
        OwnerTempleWidget->AddToViewport();
    } 
}

void UTeleportMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    UE_LOG(LogTemp, Warning, TEXT("TeleportMenu NativeOnInitialized"));

    BuildTeleportButtons();
}