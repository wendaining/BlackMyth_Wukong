#include "BlackMythPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../InteractInterface.h"
#include "../WukongCharacter.h"
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

    static ConstructorHelpers::FObjectFinder<UInputAction> TempleIA(
        TEXT("/Game/_BlackMythGame/Input/IA_Temple"));
    if (TempleIA.Succeeded()) {
        TempleAction = TempleIA.Object;
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

    // 读取 OpenLevel 传进来的参数
    if (GetWorld()->URL.HasOption(TEXT("LoadGame")))
    {
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(
            Handle,
            this,
            &ABlackMythPlayerController::EnterLoadGameFromPause,
            0.1f,
            false
        );
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

        if (TempleAction != nullptr) {
            enhanced->BindAction(TempleAction, ETriggerEvent::Triggered, this,
                &ABlackMythPlayerController::Interact);
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

void ABlackMythPlayerController::Interact()
{
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn) return;

    AWukongCharacter* Wukong = Cast<AWukongCharacter>(PlayerPawn);
    if (Wukong && Wukong->CurrentInteractable)
    {
        if (Wukong->CurrentInteractable->Implements<UInteractInterface>())
        {
            IInteractInterface::Execute_OnInteract(Wukong->CurrentInteractable, PlayerPawn);
        }
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

void ABlackMythPlayerController::EnterLoadGameFromPause()
{
    if (!PauseMenuClass) return;

    // 1. 创建 / 显示 PauseMenu
    if (!PauseMenuInstance)
    {
        PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
    }

    if (PauseMenuInstance)
    {
        PauseMenuInstance->AddToViewport();
    }

    // 2. 暂停游戏
    UGameplayStatics::SetGamePaused(GetWorld(), true);

    bShowMouseCursor = true;
    SetInputMode(FInputModeUIOnly());

    // 3. 进入读档界面
    if (UPauseMenuWidget* PauseWidget =
        Cast<UPauseMenuWidget>(PauseMenuInstance))
    {
        PauseWidget->OnLoadClicked();
    }
}
