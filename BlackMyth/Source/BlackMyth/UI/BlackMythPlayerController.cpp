#include "BlackMythPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
ABlackMythPlayerController::ABlackMythPlayerController()
    : PauseMenuInstance(nullptr)
{

    bShowMouseCursor = false;
    // 查找并缓存常用的蓝图类/对象。
    static ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuBPClass(
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/WBP_PauseMenu"));
    if (PauseMenuBPClass.Succeeded()) {
        PauseMenuClass = PauseMenuBPClass.Class;
    }

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/IMC_Player.IMC_Player"));
    if (IMC.Succeeded()) {
        PlayerMappingContext = IMC.Object;
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> IA(
        TEXT("/Game/_BlackMythGame/Blueprints/Menu/IA_Pause.IA_Pause"));
    if (IA.Succeeded()) {
        PauseAction = IA.Object;
    }
}

void ABlackMythPlayerController::BeginPlay() {
    Super::BeginPlay();

    // 如果可用，为本地玩家添加增强输入映射。
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
        if (PlayerMappingContext != nullptr) {
            Subsystem->AddMappingContext(PlayerMappingContext, 0);
        }
    }
}

void ABlackMythPlayerController::SetupInputComponent() {
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* enhanced = Cast<UEnhancedInputComponent>(InputComponent)) {
        // 绑定暂停动作（如已配置）以切换暂停菜单。
        if (PauseAction != nullptr) {
            enhanced->BindAction(PauseAction, ETriggerEvent::Triggered, this,
                                 &ABlackMythPlayerController::TogglePauseMenu);
        }
    }
}

void ABlackMythPlayerController::TogglePauseMenu(const FInputActionValue& /*Value*/) {
    if (PauseMenuClass == nullptr) {
        return;
    }

    if (PauseMenuInstance == nullptr) {
        PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
    }

    UWorld* world = GetWorld();
    if (world == nullptr) {
        return;
    }

    const bool was_paused = world->IsPaused();

    if (!was_paused) {
        // 显示暂停界面，切换为仅 UI 的输入模式。
        if (PauseMenuInstance != nullptr) {
            PauseMenuInstance->AddToViewport();
        }

        UGameplayStatics::SetGamePaused(world, true);
        bShowMouseCursor = true;

        if (PauseMenuInstance != nullptr) {
            FInputModeUIOnly input_mode;
            input_mode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
            input_mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(input_mode);
        }
    } else {
        // 隐藏暂停界面，恢复游戏输入。
        ContinueGame();
    }
}

void ABlackMythPlayerController::ContinueGame() {
    UWorld* world = GetWorld();
    if (world == nullptr) {
        return;
    }

    if (PauseMenuInstance != nullptr) {
        PauseMenuInstance->RemoveFromParent();
    }

    UGameplayStatics::SetGamePaused(world, false);
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
}