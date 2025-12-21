#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TradeMenuWidget.generated.h"

UCLASS()
class BLACKMYTH_API UTradeMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ´ÓTempleMenu´ò¿ª
    UPROPERTY()
    UUserWidget* OwnerTempleWidget;

    UFUNCTION(BlueprintCallable)
    void OnBackClicked();
};