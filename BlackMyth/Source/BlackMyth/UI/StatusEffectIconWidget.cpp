// 状态效果图标 Widget 实现

#include "StatusEffectIconWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UStatusEffectIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化进度条为满
	if (DurationProgressBar)
	{
		DurationProgressBar->SetPercent(1.0f);
	}

	// 初始化文本（如果存在）为隐藏
	if (DurationText)
	{
		DurationText->SetText(FText::FromString(TEXT("")));
	}
}

void UStatusEffectIconWidget::SetEffectType(EStatusEffectType InEffectType)
{
	EffectType = InEffectType;
	UpdateIconDisplay();
}

void UStatusEffectIconWidget::UpdateDuration(float RemainingTime, float InTotalDuration)
{
	// 更新总持续时间（首次调用时设置）
	if (TotalDuration <= 0.0f && InTotalDuration > 0.0f)
	{
		TotalDuration = InTotalDuration;
	}

	// 更新进度条
	if (DurationProgressBar && TotalDuration > 0.0f)
	{
		float Percent = FMath::Clamp(RemainingTime / TotalDuration, 0.0f, 1.0f);
		DurationProgressBar->SetPercent(Percent);

		// 根据剩余时间改变进度条颜色（低于25%时变红）
		if (Percent <= 0.25f)
		{
			DurationProgressBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f));
		}
		else
		{
			DurationProgressBar->SetFillColorAndOpacity(GetColorForEffectType(EffectType));
		}
	}

	// 更新时间文本
	if (DurationText)
	{
		// 显示剩余时间，保留一位小数
		FString TimeString = FString::Printf(TEXT("%.1f"), FMath::Max(0.0f, RemainingTime));
		DurationText->SetText(FText::FromString(TimeString));
	}
}

FLinearColor UStatusEffectIconWidget::GetColorForEffectType(EStatusEffectType Type) const
{
	switch (Type)
	{
	case EStatusEffectType::Poison:
		return PoisonColor;
	case EStatusEffectType::Slow:
		return SlowColor;
	case EStatusEffectType::Burn:
		return BurnColor;
	default:
		return DefaultColor;
	}
}

UTexture2D* UStatusEffectIconWidget::GetIconTextureForEffectType(EStatusEffectType Type) const
{
	switch (Type)
	{
	case EStatusEffectType::Poison:
		return PoisonIconTexture;
	case EStatusEffectType::Slow:
		return SlowIconTexture;
	case EStatusEffectType::Burn:
		return BurnIconTexture;
	default:
		return nullptr;
	}
}

void UStatusEffectIconWidget::UpdateIconDisplay()
{
	if (!EffectIcon)
	{
		return;
	}

	// 获取对应的颜色
	FLinearColor IconColor = GetColorForEffectType(EffectType);

	// 设置图标纹理
	UTexture2D* IconTexture = GetIconTextureForEffectType(EffectType);
	if (IconTexture)
	{
		EffectIcon->SetBrushFromTexture(IconTexture);
		EffectIcon->SetColorAndOpacity(FLinearColor::White);  // 使用纹理原色
	}
	else
	{
		// 如果没有纹理，使用纯色方块
		EffectIcon->SetBrushFromTexture(nullptr);
		EffectIcon->SetColorAndOpacity(IconColor);
	}

	// 设置进度条颜色
	if (DurationProgressBar)
	{
		DurationProgressBar->SetFillColorAndOpacity(IconColor);
	}

	UE_LOG(LogTemp, Log, TEXT("StatusEffectIconWidget: Set effect type to %d"), static_cast<int32>(EffectType));
}
