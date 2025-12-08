// 敌人头顶血条 Widget 实现

#include "EnemyHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "../Components/HealthComponent.h"

void UEnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化显示为满血
	if (HealthBar)
	{
		HealthBar->SetPercent(1.0f);
	}
}

void UEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 平滑过渡血条
	if (HealthBar && !FMath::IsNearlyEqual(CurrentDisplayPercent, TargetHealthPercent, 0.001f))
	{
		CurrentDisplayPercent = FMath::FInterpTo(CurrentDisplayPercent, TargetHealthPercent, InDeltaTime, SmoothSpeed);
		HealthBar->SetPercent(CurrentDisplayPercent);
	}
}

void UEnemyHealthBarWidget::InitializeHealthBar(UHealthComponent* InHealthComponent)
{
	HealthComponent = InHealthComponent;

	if (HealthComponent)
	{
		// 绑定生命值变化委托
		HealthComponent->OnHealthChanged.AddDynamic(this, &UEnemyHealthBarWidget::OnHealthChanged);

		UE_LOG(LogTemp, Warning, TEXT("[HealthBar] Bindded to %s, Current: %.1f/%.1f"),
			*HealthComponent->GetOwner()->GetName(),
			HealthComponent->GetCurrentHealth(),
			HealthComponent->GetMaxHealth());

		// 立即更新一次
		UpdateHealthBar();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[HealthBar] HealthComponent is NULL!"));
	}
	
}

void UEnemyHealthBarWidget::UpdateHealthBar()
{
	if (HealthComponent)
	{
		TargetHealthPercent = HealthComponent->GetHealthPercent();
	}
}

void UEnemyHealthBarWidget::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
	float Percent = CurrentHealth / MaxHealth;

	UpdateHealthBar();

	FLinearColor BarColor;

	UE_LOG(LogTemp, Warning, TEXT("[HealthBar] OnHealthChanged: %.1f/%.1f"), CurrentHealth, MaxHealth);
	// 根据血量百分比改变颜色（绿→黄→红）
	if (Percent > 0.5f)
		BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Green, (Percent - 0.5f) * 2.0f);
	else
		BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Yellow, Percent * 2.0f);

	HealthBar->SetFillColorAndOpacity(BarColor);

	UE_LOG(LogTemp, Verbose, TEXT("[EnemyHealthBar] Health Changed: %.1f/%.1f (%.1f%%)"),
		CurrentHealth, MaxHealth, TargetHealthPercent * 100.0f);
}
