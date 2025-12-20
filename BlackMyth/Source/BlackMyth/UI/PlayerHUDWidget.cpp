// 玩家 HUD Widget 实现

#include "PlayerHUDWidget.h"
#include "StatusEffectIconWidget.h"
#include "SkillBarWidget.h"
#include "InventoryBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "../Components/HealthComponent.h"
#include "../Components/StaminaComponent.h"
#include "../Components/StatusEffectComponent.h"

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

	// 默认隐藏背包栏
	if (InventoryBar)
	{
		InventoryBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::NativeDestruct()
{
	UnbindComponentDelegates();
	UnbindStatusEffectDelegates();

	// 清除计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ComboHideTimerHandle);
	}

	// 清除所有状态效果图标
	ActiveEffectIcons.Empty();
	EffectTotalDurations.Empty();

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

// ========== 状态效果相关实现 ==========

void UPlayerHUDWidget::BindStatusEffectComponent(UStatusEffectComponent* StatusEffectComponent)
{
	if (!StatusEffectComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidget::BindStatusEffectComponent - StatusEffectComponent is null!"));
		return;
	}

	// 解绑旧组件
	UnbindStatusEffectDelegates();

	// 缓存组件引用
	CachedStatusEffectComponent = StatusEffectComponent;

	// 绑定委托
	StatusEffectComponent->OnEffectApplied.AddDynamic(this, &UPlayerHUDWidget::AddStatusEffectIcon);
	StatusEffectComponent->OnEffectRemoved.AddDynamic(this, &UPlayerHUDWidget::RemoveStatusEffectIcon);
	StatusEffectComponent->OnEffectUpdated.AddDynamic(this, &UPlayerHUDWidget::UpdateStatusEffectDuration);

	UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget: Bound to StatusEffectComponent"));
}

void UPlayerHUDWidget::UnbindStatusEffectDelegates()
{
	if (CachedStatusEffectComponent.IsValid())
	{
		CachedStatusEffectComponent->OnEffectApplied.RemoveDynamic(this, &UPlayerHUDWidget::AddStatusEffectIcon);
		CachedStatusEffectComponent->OnEffectRemoved.RemoveDynamic(this, &UPlayerHUDWidget::RemoveStatusEffectIcon);
		CachedStatusEffectComponent->OnEffectUpdated.RemoveDynamic(this, &UPlayerHUDWidget::UpdateStatusEffectDuration);
	}
}

void UPlayerHUDWidget::AddStatusEffectIcon(EStatusEffectType EffectType, float Duration)
{
	// 检查容器是否存在
	if (!StatusEffectContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidget::AddStatusEffectIcon - StatusEffectContainer is null!"));
		return;
	}

	// 检查是否已经有这个效果的图标（如有则更新持续时间）
	if (ActiveEffectIcons.Contains(EffectType))
	{
		// 刷新总持续时间
		EffectTotalDurations.Add(EffectType, Duration);
		UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget: Refreshed effect icon for type %d, duration: %.1f"), static_cast<int32>(EffectType), Duration);
		return;
	}

	// 检查图标类是否设置
	if (!StatusEffectIconClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidget::AddStatusEffectIcon - StatusEffectIconClass is not set!"));
		return;
	}

	// 创建新的图标 Widget
	UStatusEffectIconWidget* NewIcon = CreateWidget<UStatusEffectIconWidget>(this, StatusEffectIconClass);
	if (!NewIcon)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerHUDWidget::AddStatusEffectIcon - Failed to create icon widget!"));
		return;
	}

	// 设置效果类型
	NewIcon->SetEffectType(EffectType);

	// 添加到容器
	UHorizontalBoxSlot* IconSlot = StatusEffectContainer->AddChildToHorizontalBox(NewIcon);
	if (IconSlot)
	{
		// 设置间距
		IconSlot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
	}

	// 保存引用和持续时间
	ActiveEffectIcons.Add(EffectType, NewIcon);
	EffectTotalDurations.Add(EffectType, Duration);

	UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget: Added effect icon for type %d, duration: %.1f"), static_cast<int32>(EffectType), Duration);
}

void UPlayerHUDWidget::RemoveStatusEffectIcon(EStatusEffectType EffectType)
{
	// 检查是否存在该图标
	UStatusEffectIconWidget** FoundIcon = ActiveEffectIcons.Find(EffectType);
	if (!FoundIcon || !(*FoundIcon))
	{
		return;
	}

	// 从容器中移除并销毁
	(*FoundIcon)->RemoveFromParent();

	// 从映射中移除
	ActiveEffectIcons.Remove(EffectType);
	EffectTotalDurations.Remove(EffectType);

	UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget: Removed effect icon for type %d"), static_cast<int32>(EffectType));
}

void UPlayerHUDWidget::UpdateStatusEffectDuration(EStatusEffectType EffectType, float RemainingTime)
{
	// 查找图标
	UStatusEffectIconWidget** FoundIcon = ActiveEffectIcons.Find(EffectType);
	if (!FoundIcon || !(*FoundIcon))
	{
		return;
	}

	// 获取总持续时间
	float TotalDuration = 0.0f;
	if (float* FoundDuration = EffectTotalDurations.Find(EffectType))
	{
		TotalDuration = *FoundDuration;
	}

	// 更新图标显示
	(*FoundIcon)->UpdateDuration(RemainingTime, TotalDuration);
}

// ========== 背包栏相关实现 ==========

void UPlayerHUDWidget::SetInventoryVisible(bool bVisible)
{
	if (InventoryBar)
	{
		InventoryBar->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Log, TEXT("PlayerHUDWidget: Inventory visibility set to %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidget::SetInventoryVisible - InventoryBar is null!"));
	}
}

