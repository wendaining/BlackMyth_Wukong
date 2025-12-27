#include "TeleportMenuWidget.h"
#include "Temple.h"
#include "EngineUtils.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"

void UTeleportMenuWidget::BuildTeleportButtons()
{
    // 检查按钮容器是否有效
    if (!ButtonContainer)
    {
        // 容错处理：尝试使用根控件作为容器（如果是CanvasPanel）
        if (UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget()))
        {
            ButtonContainer = RootCanvas;
            UE_LOG(LogTemp, Warning, TEXT("ButtonContainer未绑定，使用根CanvasPanel作为容器"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ButtonContainer为空且根控件不是CanvasPanel"));
            return;
        }
    }

    // 检查传送按钮类是否配置
    if (!TeleportButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TeleportButtonClass未配置"));
        return;
    }

    // 清理上次生成的按钮（保留地图图片和返回按钮等其他UI元素）
    for (TWeakObjectPtr<UUserWidget>& W : SpawnedTeleportButtons)
    {
        if (W.IsValid())
        {
            W->RemoveFromParent();
        }
    }
    SpawnedTeleportButtons.Reset();

    // 收集场景中所有土地庙，用于自动布局计算归一化坐标
    TArray<AInteractableActor*> Temples;
    Temples.Reserve(32);

    int32 Count = 0;
    for (TActorIterator<AInteractableActor> It(GetWorld()); It; ++It)
    {
        Temples.Add(*It);
        ++Count;
        UE_LOG(LogTemp, Verbose, TEXT("找到土地庙: %s"), *It->GetName());
    }
    UE_LOG(LogTemp, Log, TEXT("土地庙总数: %d"), Count);

    // 如果容器是CanvasPanel，则进行像素级精确定位；否则使用简单顺序布局
    if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(ButtonContainer))
    {
        // 获取地图图片尺寸，优先使用MapImage的实际尺寸
        FVector2D MapSize = FVector2D(1024.f, 768.f);  // 默认尺寸
        if (MapImage)
        {
            const FSlateBrush& Brush = MapImage->GetBrush();
            const FVector2D BrushSize = Brush.ImageSize;
            if (BrushSize.X > KINDA_SMALL_NUMBER && BrushSize.Y > KINDA_SMALL_NUMBER)
            {
                MapSize = BrushSize;
            }
        }

        // 预计算自动布局所需的世界坐标范围（XY平面）
        float MinX = FLT_MAX, MinY = FLT_MAX;
        float MaxX = -FLT_MAX, MaxY = -FLT_MAX;
        if (!bUseManualPositions)
        {
            for (AInteractableActor* T : Temples)
            {
                const FVector L = T->GetActorLocation();
                MinX = FMath::Min(MinX, L.X);
                MaxX = FMath::Max(MaxX, L.X);
                MinY = FMath::Min(MinY, L.Y);
                MaxY = FMath::Max(MaxY, L.Y);
            }
            // 防止除零错误
            if (FMath::IsNearlyEqual(MinX, MaxX)) { MaxX = MinX + 1.f; }
            if (FMath::IsNearlyEqual(MinY, MaxY)) { MaxY = MinY + 1.f; }
        }

        // 遍历所有土地庙，为每个创建传送按钮
        for (AInteractableActor* T : Temples)
        {
            if (!T)
            {
                continue;
            }

            // 创建传送按钮控件
            UTeleportButtonWidget* Button = CreateWidget<UTeleportButtonWidget>(this, TeleportButtonClass);
            if (!Button)
            {
                continue;
            }
            Button->TargetTempleID = T->TempleID;
            Button->SetVisibility(ESlateVisibility::Visible);
            UE_LOG(LogTemp, Verbose, TEXT("为土地庙生成传送按钮，ID: %s"), *T->TempleID.ToString());

            // 将按钮添加到画布并获取布局槽
            UCanvasPanelSlot* CanvasSlot = nullptr;
            if (UPanelSlot* Added = Canvas->AddChild(Button))
            {
                CanvasSlot = Cast<UCanvasPanelSlot>(Added);
            }

            if (!CanvasSlot)
            {
                // 如果无法获取CanvasSlot，则使用简单的添加方式
                ButtonContainer->AddChild(Button);
                SpawnedTeleportButtons.Add(Button);
                continue;
            }

            // 计算按钮在地图上的像素位置
            FVector2D PosPx = FVector2D::ZeroVector;

            // 根据定位模式计算按钮位置
            if (bUseManualPositions)
            {
                // 手动模式：从配置表中查找坐标
                if (const FVector2D* Found = ManualNormalizedPositions.Find(T->TempleID))
                {
                    PosPx = FVector2D(Found->X * MapSize.X, Found->Y * MapSize.Y);
                }
                else
                {
                    // 如果手动模式下未配置坐标，使用基于位置的伪随机偏移，避免重叠
                    const FVector Hash = T->GetActorLocation();
                    const float R1 = FMath::Frac(Hash.X * 0.00123f);
                    const float R2 = FMath::Frac(Hash.Y * 0.00456f);
                    PosPx = FVector2D(
                        (0.5f + (R1 - 0.5f) * 0.2f) * MapSize.X,
                        (0.5f + (R2 - 0.5f) * 0.2f) * MapSize.Y
                    );
                }
            }
            else
            {
                // 自动模式：根据世界坐标计算归一化位置
                const FVector L = T->GetActorLocation();
                const float Nx = (L.X - MinX) / (MaxX - MinX);
                float Ny = (L.Y - MinY) / (MaxY - MinY);
                if (bInvertYForAutoLayout)
                {
                    Ny = 1.f - Ny;
                }
                PosPx = FVector2D(Nx * MapSize.X, Ny * MapSize.Y);
            }

            // 配置按钮在CanvasPanel上的布局属性
            CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));  // 居中对齐
            CanvasSlot->SetAutoSize(false);
            CanvasSlot->SetSize(ButtonPixelSize);
            CanvasSlot->SetPosition(PosPx);
            CanvasSlot->SetZOrder(5);  // 设置层级，确保按钮显示在地图上方
            SpawnedTeleportButtons.Add(Button);
        }
        UE_LOG(LogTemp, Log, TEXT("在CanvasPanel上生成了 %d 个传送按钮"), SpawnedTeleportButtons.Num());
    }
    else
    {
        // 退化布局模式：容器不是CanvasPanel时使用简单的顺序添加
        for (AInteractableActor* T : Temples)
        {
            if (!T)
            {
                continue;
            }
            UTeleportButtonWidget* Button = CreateWidget<UTeleportButtonWidget>(this, TeleportButtonClass);
            if (!Button)
            {
                continue;
            }
            Button->TargetTempleID = T->TempleID;
            Button->SetVisibility(ESlateVisibility::Visible);
            ButtonContainer->AddChild(Button);
            SpawnedTeleportButtons.Add(Button);
        }
        UE_LOG(LogTemp, Log, TEXT("使用退化布局生成了 %d 个传送按钮"), SpawnedTeleportButtons.Num());
    }
}

void UTeleportMenuWidget::OnBackClicked()
{
    // 关闭传送菜单
    RemoveFromParent();
    
    // 如果有父级菜单，则重新显示
    if (OwnerTempleWidget)
    {
        OwnerTempleWidget->AddToViewport();
    }
}

void UTeleportMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    UE_LOG(LogTemp, Warning, TEXT("传送菜单初始化"));

    // 初始化时构建所有传送按钮
    BuildTeleportButtons();
}