// 玩家 HUD Widget - 显示生命值、体力值、状态效果等信息

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../StatusEffect/StatusEffectTypes.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UStaminaComponent;
class USkillBarWidget;
class UHorizontalBox;
class UStatusEffectComponent;
class UStatusEffectIconWidget;
class UInventoryBarWidget;

/**
 * 玩家 HUD Widget
 * 显示生命值、体力值、连击数、状态效果等游戏信息
 *
 * 使用方法：
 * 1. 创建继承此类的 Widget Blueprint
 * 2. 在蓝图中添加 ProgressBar，命名必须匹配 BindWidget 的变量名
 * 3. 调用 InitializeHUD() 绑定角色组件
 * 4. 可选：添加 HorizontalBox 命名为 StatusEffectContainer 用于显示状态效果图标
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

	/** 更新金币显示 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateGoldDisplay(int32 NewGold);

	/** 触发技能冷却显示（按槽位索引） */
	UFUNCTION(BlueprintCallable, Category = "HUD|Skill")
	void TriggerSkillCooldown(int32 SlotIndex, float CooldownDuration);

	/** 触发技能冷却显示（按技能名称） */
	UFUNCTION(BlueprintCallable, Category = "HUD|Skill")
	void TriggerSkillCooldownByName(const FString& SkillName, float CooldownDuration);

	/** 获取技能栏 Widget */
	UFUNCTION(BlueprintCallable, Category = "HUD|Skill")
	USkillBarWidget* GetSkillBar() const { return SkillBar; }

	// ========== 状态效果相关 ==========

	/** 绑定状态效果组件，监听效果变化事件 */
	UFUNCTION(BlueprintCallable, Category = "HUD|StatusEffect")
	void BindStatusEffectComponent(UStatusEffectComponent* StatusEffectComponent);

	/** 添加状态效果图标 */
	UFUNCTION(BlueprintCallable, Category = "HUD|StatusEffect")
	void AddStatusEffectIcon(EStatusEffectType EffectType, float Duration);

	/** 移除状态效果图标 */
	UFUNCTION(BlueprintCallable, Category = "HUD|StatusEffect")
	void RemoveStatusEffectIcon(EStatusEffectType EffectType);

	/** 更新状态效果剩余时间 */
	UFUNCTION(BlueprintCallable, Category = "HUD|StatusEffect")
	void UpdateStatusEffectDuration(EStatusEffectType EffectType, float RemainingTime);

	// ========== 背包栏相关 ==========

	/** 设置背包栏可见性 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
	void SetInventoryVisible(bool bVisible);

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

	/** 金币显示文本 - 蓝图中必须有同名 TextBlock（可选） */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* GoldText;

	/** 技能栏 Widget - 蓝图中必须有同名 Widget（可选） */
	UPROPERTY(meta = (BindWidgetOptional))
	USkillBarWidget* SkillBar;

	/** 状态效果图标容器 - 蓝图中可选的 HorizontalBox */
	UPROPERTY(meta = (BindWidgetOptional))
	UHorizontalBox* StatusEffectContainer;

	/** 背包栏 Widget - 蓝图中可选 */
	UPROPERTY(meta = (BindWidgetOptional))
	UInventoryBarWidget* InventoryBar;

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

	/** 状态效果图标 Widget 类（需要在蓝图中设置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|StatusEffect")
	TSubclassOf<UStatusEffectIconWidget> StatusEffectIconClass;

private:
	/** 缓存的组件引用 */
	UPROPERTY()
	TWeakObjectPtr<UHealthComponent> CachedHealthComponent;

	UPROPERTY()
	TWeakObjectPtr<UStaminaComponent> CachedStaminaComponent;

	/** 缓存的状态效果组件引用 */
	UPROPERTY()
	TWeakObjectPtr<UStatusEffectComponent> CachedStatusEffectComponent;

	/** 当前显示的状态效果图标 */
	UPROPERTY()
	TMap<EStatusEffectType, UStatusEffectIconWidget*> ActiveEffectIcons;

	/** 各效果的总持续时间（用于计算进度条） */
	TMap<EStatusEffectType, float> EffectTotalDurations;

	/** 连击隐藏计时器句柄 */
	FTimerHandle ComboHideTimerHandle;

	/** 绑定组件委托 */
	void BindComponentDelegates();

	/** 解绑组件委托 */
	void UnbindComponentDelegates();

	/** 隐藏连击显示 */
	void HideCombo();

	/** 解绑状态效果组件委托 */
	void UnbindStatusEffectDelegates();
};
