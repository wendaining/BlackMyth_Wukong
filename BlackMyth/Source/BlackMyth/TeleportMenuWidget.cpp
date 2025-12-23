// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportMenuWidget.h"
#include "Temple.h"
#include "EngineUtils.h"
// UI
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"

void UTeleportMenuWidget::BuildTeleportButtons()
{
    if (!ButtonContainer)
    {
        // 容错：尝试把根控件当作容器（若根是 CanvasPanel）
        if (UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget()))
        {
            ButtonContainer = RootCanvas;
            UE_LOG(LogTemp, Warning, TEXT("ButtonContainer not bound; using Root CanvasPanel as container."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ButtonContainer is NULL and root is not CanvasPanel."));
            return;
        }
    }

    if (!TeleportButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TeleportButtonClass is NULL"));
        return;
    }

    // 不再清空整个容器，避免把 MapImage、BackButton 一并删掉
    // 仅移除上次生成的按钮
    for (TWeakObjectPtr<UUserWidget>& W : SpawnedTeleportButtons)
    {
        if (W.IsValid())
        {
            W->RemoveFromParent();
        }
    }
    SpawnedTeleportButtons.Reset();

    // 收集全部庙体，便于 auto 布局计算归一化坐标
    TArray<AInteractableActor*> Temples;
    Temples.Reserve(32);

    int32 Count = 0;
    for (TActorIterator<AInteractableActor> It(GetWorld()); It; ++It)
    {
        Temples.Add(*It);
        ++Count;
        UE_LOG(LogTemp, Verbose, TEXT("Found InteractableActor: %s"), *It->GetName());
    }
    UE_LOG(LogTemp, Log, TEXT("Total InteractableActors: %d"), Count);

    // 如果是 CanvasPanel，我们进行像素级定位；否则退化为原先的顺序摆放
    if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(ButtonContainer))
    {
        // 取地图尺寸：优先 MapImage->Brush.ImageSize；若无，给个兜底值
        FVector2D MapSize = FVector2D(1024.f, 768.f);
        if (MapImage)
        {
            const FSlateBrush& Brush = MapImage->GetBrush();
            const FVector2D BrushSize = Brush.ImageSize;
            if (BrushSize.X > KINDA_SMALL_NUMBER && BrushSize.Y > KINDA_SMALL_NUMBER)
            {
                MapSize = BrushSize;
            }
        }

        // 预计算 auto 布局的世界范围（XY 平面）
        float MinX = FLT_MAX, MinY = FLT_MAX;
        float MaxX = -FLT_MAX, MaxY = -FLT_MAX;
        if (!bUseManualPositions)
        {
            for (AInteractableActor* T : Temples)
            {
                const FVector L = T->GetActorLocation();
                MinX = FMath::Min(MinX, L.X); MaxX = FMath::Max(MaxX, L.X);
                MinY = FMath::Min(MinY, L.Y); MaxY = FMath::Max(MaxY, L.Y);
            }
            // 防止除零
            if (FMath::IsNearlyEqual(MinX, MaxX)) { MaxX = MinX + 1.f; }
            if (FMath::IsNearlyEqual(MinY, MaxY)) { MaxY = MinY + 1.f; }
        }

        for (AInteractableActor* T : Temples)
        {
            if (!T) continue;

            UTeleportButtonWidget* Button = CreateWidget<UTeleportButtonWidget>(GetWorld(), TeleportButtonClass);
            if (!Button) continue;
            Button->TargetTempleID = T->TempleID;
            Button->SetVisibility(ESlateVisibility::Visible);
            UE_LOG(LogTemp, Verbose, TEXT("Spawn TeleportButton for TempleID %s"), *T->TempleID.ToString());

            UCanvasPanelSlot* CanvasSlot = nullptr;
            if (UPanelSlot* Added = Canvas->AddChild(Button))
            {
                CanvasSlot = Cast<UCanvasPanelSlot>(Added);
            }

            if (!CanvasSlot)
            {
                // 失败则直接加入容器（不定位）
                ButtonContainer->AddChild(Button);
                SpawnedTeleportButtons.Add(Button);
                continue;
            }

            // 计算在地图上的像素位置
            FVector2D PosPx = FVector2D::ZeroVector;

            if (bUseManualPositions)
            {
                if (const FVector2D* Found = ManualNormalizedPositions.Find(T->TempleID))
                {
                    PosPx = FVector2D(Found->X * MapSize.X, Found->Y * MapSize.Y);
                }
                else
                {
                    // 手动模式却未提供坐标：退化为居中随机略偏，避免全部重叠
                    const FVector Hash = T->GetActorLocation();
                    const float R1 = FMath::Frac(Hash.X * 0.00123f);
                    const float R2 = FMath::Frac(Hash.Y * 0.00456f);
                    PosPx = FVector2D((0.5f + (R1 - 0.5f) * 0.2f) * MapSize.X,
                                      (0.5f + (R2 - 0.5f) * 0.2f) * MapSize.Y);
                }
            }
            else
            {
                const FVector L = T->GetActorLocation();
                const float Nx = (L.X - MinX) / (MaxX - MinX);
                float Ny = (L.Y - MinY) / (MaxY - MinY);
                if (bInvertYForAutoLayout)
                {
                    Ny = 1.f - Ny;
                }
                PosPx = FVector2D(Nx * MapSize.X, Ny * MapSize.Y);
            }

            // 在 CanvasPanel 上定位与定尺寸（居中对齐）
            CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            CanvasSlot->SetAutoSize(false);
            CanvasSlot->SetSize(ButtonPixelSize);
            CanvasSlot->SetPosition(PosPx);
            CanvasSlot->SetZOrder(5); // 确保按钮在地图上方
            SpawnedTeleportButtons.Add(Button);
        }
        UE_LOG(LogTemp, Log, TEXT("Spawned %d teleport buttons on CanvasPanel."), SpawnedTeleportButtons.Num());
    }
    else
    {
        // 退化布局：按容器原有规则纵向/网格添加
        for (AInteractableActor* T : Temples)
        {
            if (!T) continue;
            UTeleportButtonWidget* Button = CreateWidget<UTeleportButtonWidget>(GetWorld(), TeleportButtonClass);
            if (!Button) continue;
            Button->TargetTempleID = T->TempleID;
            Button->SetVisibility(ESlateVisibility::Visible);
            ButtonContainer->AddChild(Button);
            SpawnedTeleportButtons.Add(Button);
        }
        UE_LOG(LogTemp, Log, TEXT("Spawned %d teleport buttons in fallback layout."), SpawnedTeleportButtons.Num());
    }
}

void UTeleportMenuWidget::OnBackClicked() {
    RemoveFromParent();
    if (OwnerTempleWidget) {
        OwnerTempleWidget->AddToViewport();
    } 
}

void UTeleportMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    UE_LOG(LogTemp, Warning, TEXT("TeleportMenu NativeOnInitialized"));

    BuildTeleportButtons();
}