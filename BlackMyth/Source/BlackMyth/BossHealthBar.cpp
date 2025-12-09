#include "BossHealthBar.h"
#include "Components/ProgressBar.h"

void UBossHealthBar::InitializeWidget(UHealthComponent* NewHealthComponent)
{
	HealthComponent = NewHealthComponent;
}

void UBossHealthBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HealthComponent && HealthProgressBar)
	{
		// 实时更新进度条百分比
		HealthProgressBar->SetPercent(HealthComponent->GetHealthPercent());
	}
}
