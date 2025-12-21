// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportMenuWidget.h"

void UTeleportMenuWidget::OnBackClicked()
{
    RemoveFromParent();

    if (OwnerTempleWidget) {
        OwnerTempleWidget->AddToViewport();
    }
}