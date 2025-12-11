#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "BlackMythPlayerController.generated.h"

class UUserWidget;
struct FInputActionValue;

/**
 * ???????????? Tab ??/?????????
 */
UCLASS()
class BLACKMYTH_API ABlackMythPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ABlackMythPlayerController();

    // ???????
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* PlayerMappingContext;

    // Pause Action?????????
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PauseAction;

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // ????????????????
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuClass;

    // ?????????
    UPROPERTY()
    UUserWidget* PauseMenuInstance;

    // ?§Ý???????
    void TogglePauseMenu(const FInputActionValue& Value);

};
