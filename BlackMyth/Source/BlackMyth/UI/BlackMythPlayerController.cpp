#include "BlackMythPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../InteractInterface.h"
#include "../WukongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LevelStreaming.h"
#include "Algo/AllOf.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
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

    // 等待流式关卡就绪后再启用重力和碰撞，避免出生时掉落穿地。
    BeginDeferredSpawnProtection();
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

void ABlackMythPlayerController::BeginDeferredSpawnProtection()
{
    if (bSpawnProtectionActive)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* Move = Wukong->GetCharacterMovement())
        {
            CachedGravityScale = Move->GravityScale;
            CachedMovementMode = Move->MovementMode;
            Move->SetMovementMode(MOVE_Flying);
            Move->GravityScale = 0.0f;
            Move->Velocity = FVector::ZeroVector;
        }

        Wukong->SetActorEnableCollision(false);
    }

    bSpawnProtectionActive = true;
    SpawnProtectionStartTime = World->GetTimeSeconds();

    World->GetTimerManager().SetTimer(
        StreamingCheckHandle,
        this,
        &ABlackMythPlayerController::TryEnablePawnAfterStreaming,
        0.2f,
        true);
}

void ABlackMythPlayerController::TryEnablePawnAfterStreaming()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    bool bReadyToEnable = false;

    // 条件1：脚下已有可行走平面，优先使用
    if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(GetPawn()))
    {
        FVector SafeLoc;
        if (ProbeGroundAndGetSafeLocation(Wukong, SafeLoc))
        {
            Wukong->SetActorLocation(SafeLoc, false, nullptr, ETeleportType::TeleportPhysics);
            bReadyToEnable = true;
        }
    }

    // 条件2：所有流式关卡已可见且加载完成
    if (!bReadyToEnable)
    {
        const bool bStreamingReady = Algo::AllOf(
            World->GetStreamingLevels(),
            [](const ULevelStreaming* Level)
            {
                return Level == nullptr || (Level->IsLevelLoaded() && Level->IsLevelVisible());
            });
        bReadyToEnable = bStreamingReady;
    }

    // 条件3：超时兜底，避免长时间悬浮
    if (!bReadyToEnable && (World->GetTimeSeconds() - SpawnProtectionStartTime) >= MaxSpawnProtectionDuration)
    {
        bReadyToEnable = true;
    }

    if (!bReadyToEnable)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(StreamingCheckHandle);

    if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* Move = Wukong->GetCharacterMovement())
        {
            Move->GravityScale = CachedGravityScale;
            Move->SetMovementMode(MOVE_Walking);
        }

        Wukong->SetActorEnableCollision(true);
    }

    bSpawnProtectionActive = false;
}

bool ABlackMythPlayerController::ProbeGroundAndGetSafeLocation(AWukongCharacter* Wukong, FVector& OutSafeLocation) const
{
    if (!Wukong)
    {
        return false;
    }

    const UCapsuleComponent* Capsule = Wukong->GetCapsuleComponent();
    const UWorld* World = GetWorld();
    if (!Capsule || !World)
    {
        return false;
    }

    const float Radius = Capsule->GetScaledCapsuleRadius();
    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

    const FVector Start = Wukong->GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, GroundProbeDistance);

    FCollisionQueryParams Params(TEXT("SpawnProbe"), false, Wukong);
    FCollisionResponseParams RespParams;
    FHitResult Hit;
    const FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

    // 使用可见性通道或世界静态通道均可，根据项目碰撞配置选择。
    const ECollisionChannel Channel = ECollisionChannel::ECC_Visibility;

    const bool bHit = World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, Channel, Shape, Params, RespParams);
    if (!bHit || !Hit.bBlockingHit)
    {
        return false;
    }

    // 简单判定可行走坡度：法线Z分量足够大（避免墙面命中）。
    if (Hit.ImpactNormal.Z < 0.5f)
    {
        return false;
    }

    // 计算安全落点：命中点上抬胶囊半高，并留出微小余量。
    OutSafeLocation = Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 2.f);
    return true;
}
