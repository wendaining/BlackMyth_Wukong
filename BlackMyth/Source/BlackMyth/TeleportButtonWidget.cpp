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

    // 隐藏土地庙名称文本，仅显示按钮
    if (TempleNameText)
    {
        TempleNameText->SetVisibility(ESlateVisibility::Collapsed);
    }

    // 绑定按钮点击事件
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
    if (!World)
    {
        return;
    }

    // 获取玩家角色
    AWukongCharacter* Player = Cast<AWukongCharacter>(
        UGameplayStatics::GetPlayerCharacter(World, 0)
    );

    if (!Player)
    {
        return;
    }

    // 遍历场景中的所有土地庙，查找目标土地庙
    for (TActorIterator<AInteractableActor> It(World); It; ++It)
    {
        // 找到匹配的土地庙
        if (It->TempleID == TargetTempleID && It->TeleportPoint)
        {
            FVector TeleportLoc = It->TeleportPoint->GetComponentLocation();
            FRotator TeleportRot = It->TeleportPoint->GetComponentRotation();
            
            // 在传送点周围随机偏移位置，避免与土地庙模型碰撞
            const float RandomRadius = FMath::RandRange(100.f, 200.f);
            const float RandomAngle = FMath::RandRange(0.f, 2.f * PI);
            const float OffsetX = RandomRadius * FMath::Cos(RandomAngle);
            const float OffsetY = RandomRadius * FMath::Sin(RandomAngle);
            const float HeightOffset = 150.f;  // 向上偏移，防止卡入地形
            
            TeleportLoc += FVector(OffsetX, OffsetY, HeightOffset);
            
            // 传送玩家到目标位置
            Player->SetActorLocation(TeleportLoc);
            Player->SetActorRotation(TeleportRot);

            // 关闭传送菜单UI
            // 关闭传送菜单和父级土地庙菜单
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

            // 恢复游戏运行状态
            UGameplayStatics::SetGamePaused(World, false);

            // 恢复游戏输入模式，隐藏鼠标光标
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

    // 在构造预览时隐藏文本标签
    if (TempleNameText)
    {
        TempleNameText->SetVisibility(ESlateVisibility::Collapsed);
    }
}