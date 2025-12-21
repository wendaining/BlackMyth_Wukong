#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TeleportMenuWidget.generated.h"

UCLASS()
class BLACKMYTH_API UTeleportMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ´ÓTempleMenu´ò¿ª
    UPROPERTY()
    UUserWidget* OwnerTempleWidget;

    UFUNCTION(BlueprintCallable)
    void OnBackClicked();
};

