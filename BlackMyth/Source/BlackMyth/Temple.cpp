#include "Temple.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "WukongCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/BlackMythPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"

AInteractableActor::AInteractableActor()
{
    // 土地庙不需要每帧更新
    PrimaryActorTick.bCanEverTick = false;

    // 创建根组件
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // 创建静态网格体组件（土地庙模型）
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);

    // 创建传送点组件（玩家传送到此位置）
    TeleportPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TeleportPoint"));
    TeleportPoint->SetupAttachment(RootComponent);

    // 创建交互范围球体组件
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();

    // 绑定交互范围的重叠事件
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor::OnPlayerEnter);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor::OnPlayerExit);
}

void AInteractableActor::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    // 检查是否是玩家角色进入交互范围
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        // 显示交互提示UI
        if (TempleWidgetClass)
        {
            InteractWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TempleWidgetClass);
            if (InteractWidgetInstance)
            {
                InteractWidgetInstance->AddToViewport();
            }
        }

        // 设置玩家的当前可交互对象
        AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
        if (Player)
        {
            Player->CurrentInteractable = this;
        }
    }
}

void AInteractableActor::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 检查是否是玩家角色离开交互范围
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        // 移除交互提示UI
        if (InteractWidgetInstance)
        {
            InteractWidgetInstance->RemoveFromParent();
            InteractWidgetInstance = nullptr;
        }

        // 移除交互菜单UI
        if (InteractMenuInstance)
        {
            InteractMenuInstance->RemoveFromParent();
            InteractMenuInstance = nullptr;
        }

        // 清除玩家的当前可交互对象引用
        AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
        if (Player && Player->CurrentInteractable == this)
        {
            Player->CurrentInteractable = nullptr;
        }
    }
}

void AInteractableActor::DoInteract()
{
    UE_LOG(LogTemp, Warning, TEXT("与 %s 发生交互"), *GetName());
}

void AInteractableActor::OnInteract_Implementation(AActor* Interactor)
{
    // 验证交互者是否为玩家角色
    AWukongCharacter* Player = Cast<AWukongCharacter>(Interactor);
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("[土地庙] 交互者不是悟空角色"));
        return;
    }

    // 检查交互菜单蓝图是否配置
    if (!InteractMenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractMenuWidgetClass未配置"));
        return;
    }

    // 防止重复打开菜单
    if (InteractMenuInstance)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    // 创建并显示土地庙交互菜单
    InteractMenuInstance = CreateWidget<UUserWidget>(PC, InteractMenuWidgetClass);
    if (InteractMenuInstance)
    {
        InteractMenuInstance->AddToViewport(100);

        // 暂停游戏
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        // 切换到UI输入模式，显示鼠标光标
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;

        UE_LOG(LogTemp, Log, TEXT("土地庙交互菜单已打开"));
    }

    // 完全恢复玩家的血量和体力
    Player->FullRestore();
}