#include "BlackMythPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ABlackMythPlayerController::ABlackMythPlayerController()
{
    bShowMouseCursor = false;

    static ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuBPClass(TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_PauseMenu"));
    if (PauseMenuBPClass.Succeeded())
    {
        PauseMenuClass = PauseMenuBPClass.Class;
    }

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(TEXT("/Game/_BlackMythGame/Blueprints/Menu/IMC_Player.IMC_Player"));
    if (IMC.Succeeded())
    {
        PlayerMappingContext = IMC.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> IA(TEXT("/Game/_BlackMythGame/Blueprints/Menu/IA_Pause.IA_Pause"));
    if (IA.Succeeded())
    {
        PauseAction = IA.Object;
    }
}

void ABlackMythPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // ������ǿ����ӳ��
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (PlayerMappingContext)
        {
            Subsystem->AddMappingContext(PlayerMappingContext, 0);
        }
    }
}

void ABlackMythPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent);
    if (EI)
    {
        // Tab������ PauseAction
        if (PauseAction)
        {
            EI->BindAction(PauseAction, ETriggerEvent::Triggered, this, &ABlackMythPlayerController::TogglePauseMenu);
        }
    }
}

void ABlackMythPlayerController::TogglePauseMenu(const FInputActionValue& Value)
{
    if (!PauseMenuClass)
        return;

    if (!PauseMenuInstance)
    {
        PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
    }

    UWorld* World = GetWorld();
    if (!World) return;

    bool bPaused = World->IsPaused();

    if (!bPaused)
    {
        PauseMenuInstance->AddToViewport();
        UGameplayStatics::SetGamePaused(World, true);

        bShowMouseCursor = true;
        // 设置�? UI 模式并把焦点给暂停菜�?
        if (PauseMenuInstance)
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);
        }
        else
        {
            SetInputMode(FInputModeUIOnly());
        }
    }
    else
    {
        PauseMenuInstance->RemoveFromParent();
        UGameplayStatics::SetGamePaused(World, false);

        bShowMouseCursor = false;
        FInputModeGameOnly GameInputMode;
        SetInputMode(GameInputMode);
    }
}

