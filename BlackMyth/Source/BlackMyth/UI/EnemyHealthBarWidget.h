// 敌人头顶血条 Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UProgressBar;
class UHealthComponent;

/**
 * 敌人头顶血条 Widget
 * 挂载在每个敌人头顶，实时显示生命值
 */
UCLASS()
class BLACKMYTH_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 初始化血条，绑定到指定的 HealthComponent
	 * @param InHealthComponent 要绑定的生命值组件
	 */
	UFUNCTION(BlueprintCallable, Category = "Health Bar")
	void InitializeHealthBar(UHealthComponent* InHealthComponent);

	/** 更新血条显示 */
	UFUNCTION(BlueprintCallable, Category = "Health Bar")
	void UpdateHealthBar();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 生命值变化回调 */
	UFUNCTION()
	void OnHealthChanged(float CurrentHealth, float MaxHealth);

protected:
	/** 血条进度条控件（需在蓝图中绑定同名控件） */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/** 绑定的生命值组件 */
	UPROPERTY(BlueprintReadOnly, Category = "Health Bar")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** 目标血量百分比（用于平滑过渡） */
	float TargetHealthPercent = 1.0f;

	/** 当前显示的血量百分比 */
	float CurrentDisplayPercent = 1.0f;

	/** 血条变化平滑速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	float SmoothSpeed = 5.0f;
};
