// 玩家 HUD Widget - 显示生命值、体力值等信息

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UStaminaComponent;

/**
 * 玩家 HUD Widget
 * 显示生命值、体力值、连击数等游戏信息
 * 
 * 使用方法：
 * 1. 创建继承此类的 Widget Blueprint
 * 2. 在蓝图中添加 ProgressBar，命名必须匹配 BindWidget 的变量名
 * 3. 调用 InitializeHUD() 绑定角色组件
 */
UCLASS()
class BLACKMYTH_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 初始化 HUD，绑定到角色的组件 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeHUD(ACharacter* PlayerCharacter);

	/** 更新生命值显示 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);

	/** 更新体力值显示 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateStaminaBar(float CurrentStamina, float MaxStamina);

	/** 更新连击显示 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateComboCount(int32 ComboCount);

	/** 显示/隐藏连击计数 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetComboVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ========== UI 控件绑定 ==========
	// 这些控件会自动绑定到蓝图中同名的控件

	/** 生命值进度条 - 蓝图中必须有同名 ProgressBar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	/** 体力值进度条 - 蓝图中必须有同名 ProgressBar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	/** 连击计数文本 - 蓝图中必须有同名 TextBlock（可选） */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ComboText;

	// ========== 可配置属性 ==========

	/** 生命值条颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FLinearColor HealthBarColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);  // 红色

	/** 体力值条颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FLinearColor StaminaBarColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);  // 绿色

	/** 低生命值警告阈值 (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.25f;

	/** 低生命值时的颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Style")
	FLinearColor LowHealthColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);  // 亮红色

	/** 连击显示持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Combo")
	float ComboDisplayDuration = 2.0f;

private:
	/** 缓存的组件引用 */
	UPROPERTY()
	TWeakObjectPtr<UHealthComponent> CachedHealthComponent;

	UPROPERTY()
	TWeakObjectPtr<UStaminaComponent> CachedStaminaComponent;

	/** 连击隐藏计时器句柄 */
	FTimerHandle ComboHideTimerHandle;

	/** 绑定组件委托 */
	void BindComponentDelegates();

	/** 解绑组件委托 */
	void UnbindComponentDelegates();

	/** 隐藏连击显示 */
	void HideCombo();
};
