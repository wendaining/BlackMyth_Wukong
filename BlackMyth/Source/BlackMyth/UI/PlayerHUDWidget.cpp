// 玩家 HUD Widget 实现

#include "PlayerHUDWidget.h"
#include "SkillBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "../Components/HealthComponent.h"
#include "../Components/StaminaComponent.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置初始颜色
	if (HealthBar)
	{
		HealthBar->SetFillColorAndOpacity(HealthBarColor);
	}
	if (StaminaBar)
	{
		StaminaBar->SetFillColorAndOpacity(StaminaBarColor);
	}
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::NativeDestruct()
{
	UnbindComponentDelegates();
	
	// 清除计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ComboHideTimerHandle);
	}

	Super::NativeDestruct();
}

void UPlayerHUDWidget::InitializeHUD(ACharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidget::InitializeHUD - PlayerCharacter is null!"));
		return;
	}

	// 获取组件
	CachedHealthComponent = PlayerCharacter->FindComponentByClass<UHealthComponent>();
	CachedStaminaComponent = PlayerCharacter->FindComponentByClass<UStaminaComponent>();

	// 绑定委托
	BindComponentDelegates();

	// 初始化显示
	if (CachedHealthComponent.IsValid())
	{
		UpdateHealthBar(CachedHealthComponent->GetCurrentHealth(), CachedHealthComponent->GetMaxHealth());
	}
	if (CachedStaminaComponent.IsValid())
	{
		UpdateStaminaBar(CachedStaminaComponent->GetCurrentStamina(), CachedStaminaComponent->GetMaxStamina());
	}

	UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget initialized for %s"), *PlayerCharacter->GetName());
}

void UPlayerHUDWidget::BindComponentDelegates()
{
	if (CachedHealthComponent.IsValid())
	{
		CachedHealthComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateHealthBar);
	}
	if (CachedStaminaComponent.IsValid())
	{
		CachedStaminaComponent->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateStaminaBar);
	}
}

void UPlayerHUDWidget::UnbindComponentDelegates()
{
	if (CachedHealthComponent.IsValid())
	{
		CachedHealthComponent->OnHealthChanged.RemoveDynamic(this, &UPlayerHUDWidget::UpdateHealthBar);
	}
	if (CachedStaminaComponent.IsValid())
	{
		CachedStaminaComponent->OnStaminaChanged.RemoveDynamic(this, &UPlayerHUDWidget::UpdateStaminaBar);
	}
}

void UPlayerHUDWidget::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	if (!HealthBar)
	{
		return;
	}

	float Percent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
	HealthBar->SetPercent(Percent);

	// 低生命值警告 - 改变颜色
	if (Percent <= LowHealthThreshold)
	{
		HealthBar->SetFillColorAndOpacity(LowHealthColor);
	}
	else
	{
		HealthBar->SetFillColorAndOpacity(HealthBarColor);
	}
}

void UPlayerHUDWidget::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
	if (!StaminaBar)
	{
		return;
	}

	float Percent = MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
	StaminaBar->SetPercent(Percent);
}

void UPlayerHUDWidget::UpdateComboCount(int32 ComboCount)
{
	if (!ComboText)
	{
		return;
	}

	if (ComboCount > 1)
	{
		ComboText->SetText(FText::FromString(FString::Printf(TEXT("%d Hit!"), ComboCount)));
		ComboText->SetVisibility(ESlateVisibility::Visible);

		// 重置隐藏计时器
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ComboHideTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				ComboHideTimerHandle,
				this,
				&UPlayerHUDWidget::HideCombo,
				ComboDisplayDuration,
				false
			);
		}
	}
	else
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::SetComboVisible(bool bVisible)
{
	if (ComboText)
	{
		ComboText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::HideCombo()
{
	if (ComboText)
	{
		ComboText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::TriggerSkillCooldown(int32 SlotIndex, float CooldownDuration)
{
	if (SkillBar)
	{
		SkillBar->TriggerSkillCooldown(SlotIndex, CooldownDuration);
	}
}

void UPlayerHUDWidget::TriggerSkillCooldownByName(const FString& SkillName, float CooldownDuration)
{
	if (SkillBar)
	{
		SkillBar->TriggerSkillCooldownByName(SkillName, CooldownDuration);
	}
}
