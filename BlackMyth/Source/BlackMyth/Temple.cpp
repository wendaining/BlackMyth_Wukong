// Fill out your copyright notice in the Description page of Project Settings.

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
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);

    TeleportPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TeleportPoint"));
    TeleportPoint->SetupAttachment(RootComponent);


    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor::OnPlayerEnter);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor::OnPlayerExit);
}

void AInteractableActor::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        if (TempleWidgetClass)
        {
            InteractWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TempleWidgetClass);
            if (InteractWidgetInstance)
            {
                InteractWidgetInstance->AddToViewport();
            }
        }

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
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        // 移除交互提示Widget
        if (InteractWidgetInstance)
        {
            InteractWidgetInstance->RemoveFromParent();
            InteractWidgetInstance = nullptr;
        }

        // 移除交互菜单Widget
        if (InteractMenuInstance)
        {
            InteractMenuInstance->RemoveFromParent();
            InteractMenuInstance = nullptr;
        }

        AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
        if (Player && Player->CurrentInteractable == this)
        {
            Player->CurrentInteractable = nullptr;
        }
    }
}

void AInteractableActor::DoInteract()
{
    UE_LOG(LogTemp, Warning, TEXT("Interacted with %s"), *GetName());
}

void AInteractableActor::OnInteract_Implementation(AActor* Interactor)
{
    AWukongCharacter* Player = Cast<AWukongCharacter>(Interactor);
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Temple] Interactor is not WukongCharacter"));
        return;
    }

    // ===== 1. 打开土地庙 WBP =====
    if (!InteractMenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractMenuWidgetClass is NULL"));
        return;
    }

    // 防止重复打开
    if (InteractMenuInstance)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    InteractMenuInstance = CreateWidget<UUserWidget>(PC, InteractMenuWidgetClass);
    if (InteractMenuInstance)
    {
        InteractMenuInstance->AddToViewport(100);

        // ===== 2. 暂停游戏 =====
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        // ===== 3. 切换到 UI 输入 =====
        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;

        UE_LOG(LogTemp, Log, TEXT("Temple Interact Menu Opened"));
    }

    // ===== 4. 恢复血量和体力值 =====
    Player->FullRestore();
}