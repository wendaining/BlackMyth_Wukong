// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Temple.h"
#include "WukongCharacter.h"
#include "TeleportMenuWidget.h"
#include "GameFramework/PlayerController.h"

void UTeleportButtonWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 显示 TempleID
    if (TempleNameText)
    {
        TempleNameText->SetText(FText::FromName(TargetTempleID));
    }

    if (TeleportButton)
    {
        TeleportButton->OnClicked.AddDynamic(
            this, &UTeleportButtonWidget::OnTeleportClicked
        );
    }
}

void UTeleportButtonWidget::OnTeleportClicked()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 1. 找玩家
    AWukongCharacter* Player =
        Cast<AWukongCharacter>(
            UGameplayStatics::GetPlayerCharacter(World, 0)
        );

    if (!Player) return;

    // 2. 找目标 Temple
    for (TActorIterator<AInteractableActor> It(World); It; ++It)
    {
        if (It->TempleID == TargetTempleID && It->TeleportPoint)
        {
            FVector TeleportLoc = It->TeleportPoint->GetComponentLocation();
            FRotator TeleportRot = It->TeleportPoint->GetComponentRotation();
            
            // 在传送点周围随机偏移，避免与土地庙模型重合
            const float RandomRadius = FMath::RandRange(100.f, 200.f);
            const float RandomAngle = FMath::RandRange(0.f, 2.f * PI);
            const float OffsetX = RandomRadius * FMath::Cos(RandomAngle);
            const float OffsetY = RandomRadius * FMath::Sin(RandomAngle);
            const float HeightOffset = 150.f; // 抬高避免卡地形
            
            TeleportLoc += FVector(OffsetX, OffsetY, HeightOffset);
            
            Player->SetActorLocation(TeleportLoc);
            Player->SetActorRotation(TeleportRot);

            // 3. 关闭菜单、解除暂停、恢复游戏输入
            if (UTeleportMenuWidget* TeleportMenu = GetTypedOuter<UTeleportMenuWidget>())
            {
                if (TeleportMenu->OwnerTempleWidget)
                {
                    TeleportMenu->OwnerTempleWidget->RemoveFromParent();
                }
                TeleportMenu->RemoveFromParent();
            }
            else if (UUserWidget* OwnerWidget = GetTypedOuter<UUserWidget>())
            {
                OwnerWidget->RemoveFromParent();
            }

            UGameplayStatics::SetGamePaused(World, false);

            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                PC->SetInputMode(FInputModeGameOnly());
                PC->bShowMouseCursor = false;
            }
            break;
        }
    }
}

void UTeleportButtonWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (!TempleNameText)
    {
        return;
    }
    else {
        TempleNameText->SetText(
            FText::FromName(TargetTempleID)
        );
    }

    // === 蓝图：TargetTempleID → NameToString ===
    FString NameString = TargetTempleID.ToString();

    // === 蓝图：StringToText ===
    FText DisplayText = FText::FromString(NameString);

    // === 蓝图：SetText ===
    TempleNameText->SetText(DisplayText);
}