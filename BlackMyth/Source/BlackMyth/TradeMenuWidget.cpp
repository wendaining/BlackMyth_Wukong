// Fill out your copyright notice in the Description page of Project Settings.


#include "TradeMenuWidget.h"

void UTradeMenuWidget::OnBackClicked()
{
    RemoveFromParent();

    if (OwnerTempleWidget) {
        OwnerTempleWidget->AddToViewport();
    }
}