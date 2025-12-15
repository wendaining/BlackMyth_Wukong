// 技能槽位 Widget 实现

#include "SkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化为可用状态
	UpdateVisuals();
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 更新冷却
	if (bIsOnCooldown)
	{
		UpdateCooldown(InDeltaTime);
	}
}

void USkillSlotWidget::InitializeSlot(const FString& InSkillName, const FString& InKeyName, UTexture2D* InIcon)
{
	SkillName = InSkillName;
	KeyName = InKeyName;

	// 设置按键文本
	if (KeyText)
	{
		KeyText->SetText(FText::FromString(InKeyName));
	}

	// 设置图标
	if (SkillIcon && InIcon)
	{
		SkillIcon->SetBrushFromTexture(InIcon);
	}

	// 初始状态
	bIsOnCooldown = false;
	RemainingCooldown = 0.0f;
	TotalCooldown = 0.0f;

	UpdateVisuals();
}

void USkillSlotWidget::StartCooldown(float CooldownDuration)
{
	if (CooldownDuration <= 0.0f)
	{
		return;
	}

	bIsOnCooldown = true;
	TotalCooldown = CooldownDuration;
	RemainingCooldown = CooldownDuration;

	UpdateVisuals();
}

void USkillSlotWidget::UpdateCooldown(float DeltaTime)
{
	if (!bIsOnCooldown)
	{
		return;
	}

	RemainingCooldown -= DeltaTime;

	if (RemainingCooldown <= 0.0f)
	{
		// 冷却结束
		ResetCooldown();
	}
	else
	{
		// 更新显示
		UpdateVisuals();
	}
}

void USkillSlotWidget::ResetCooldown()
{
	bIsOnCooldown = false;
	RemainingCooldown = 0.0f;
	TotalCooldown = 0.0f;

	UpdateVisuals();
}

float USkillSlotWidget::GetCooldownProgress() const
{
	if (!bIsOnCooldown || TotalCooldown <= 0.0f)
	{
		return 1.0f;  // 完全可用
	}

	return 1.0f - (RemainingCooldown / TotalCooldown);
}

void USkillSlotWidget::UpdateVisuals()
{
	if (bIsOnCooldown)
	{
		// 冷却中：灰色 + 显示倒计时
		if (SkillBackground)
		{
			SkillBackground->SetColorAndOpacity(CooldownColor);
		}

		if (SkillIcon)
		{
			SkillIcon->SetColorAndOpacity(CooldownColor);
		}

		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Visible);
			// 设置遮罩透明度基于冷却进度
			float Alpha = 1.0f - GetCooldownProgress();
			CooldownOverlay->SetRenderOpacity(Alpha * 0.7f);
		}

		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Visible);
			// 显示剩余秒数（向上取整）
			int32 SecondsRemaining = FMath::CeilToInt(RemainingCooldown);
			CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%d"), SecondsRemaining)));
		}

		if (CooldownBar)
		{
			CooldownBar->SetVisibility(ESlateVisibility::Visible);
			CooldownBar->SetPercent(GetCooldownProgress());
			CooldownBar->SetFillColorAndOpacity(CooldownBarColor);
		}
	}
	else
	{
		// 可用状态：有颜色
		if (SkillBackground)
		{
			SkillBackground->SetColorAndOpacity(AvailableColor);
		}

		if (SkillIcon)
		{
			SkillIcon->SetColorAndOpacity(FLinearColor::White);
		}

		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		}

		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}

		if (CooldownBar)
		{
			CooldownBar->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
