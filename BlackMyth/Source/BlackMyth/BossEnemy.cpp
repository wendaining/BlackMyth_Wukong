#include "BossEnemy.h"
#include "BossHealthBar.h"
#include "Kismet/GameplayStatics.h"

ABossEnemy::ABossEnemy()
{
	// Boss 通常体型较大，可以在这里调整胶囊体或移动速度
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 创建 Boss 血条 UI
	if (BossHealthBarClass)
	{
		BossHealthBarWidget = CreateWidget<UBossHealthBar>(GetWorld(), BossHealthBarClass);
		if (BossHealthBarWidget)
		{
			// 初始化 Widget，传入 HealthComponent
			BossHealthBarWidget->InitializeWidget(HealthComponent);
			
			BossHealthBarWidget->AddToViewport();
			BossHealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABossEnemy::SetBossHealthVisibility(bool bVisible)
{
	if (BossHealthBarWidget)
	{
		BossHealthBarWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
