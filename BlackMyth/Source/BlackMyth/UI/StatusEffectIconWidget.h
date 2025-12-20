// 状态效果图标 Widget - 显示单个状态效果的图标和倒计时

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../StatusEffect/StatusEffectTypes.h"
#include "StatusEffectIconWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;

/**
 * 状态效果图标 Widget
 * 显示单个状态效果的图标、进度条和剩余时间
 */
UCLASS()
class BLACKMYTH_API UStatusEffectIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 设置效果类型并更新图标 */
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void SetEffectType(EStatusEffectType InEffectType);

	/** 更新剩余时间显示 */
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void UpdateDuration(float RemainingTime, float TotalDuration);

	/** 获取当前效果类型 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	EStatusEffectType GetEffectType() const { return EffectType; }

	/** 获取总持续时间 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	float GetTotalDuration() const { return TotalDuration; }

protected:
	virtual void NativeConstruct() override;

	// ========== UI 控件绑定 ==========

	/** 效果图标 - 蓝图中必须有同名 Image */
	UPROPERTY(meta = (BindWidget))
	UImage* EffectIcon;

	/** 剩余时间进度条 - 蓝图中必须有同名 ProgressBar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* DurationProgressBar;

	/** 剩余时间文本 - 蓝图中可选的 TextBlock */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DurationText;

	/** 效果名称文本 - 蓝图中可选的 TextBlock */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* EffectNameText;

	// ========== 可配置属性 ==========

	/** 中毒效果图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor PoisonColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);  // 绿色

	/** 减速效果图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor SlowColor = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);  // 蓝色

	/** 灼烧效果图标颜色（预留） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor BurnColor = FLinearColor(1.0f, 0.4f, 0.0f, 1.0f);  // 橙色

	/** 攻击Buff图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor AttackBuffColor = FLinearColor(1.0f, 0.3f, 0.2f, 1.0f);  // 红色

	/** 防御Buff图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor DefenseBuffColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);  // 金色

	/** 血量恢复图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor HealingIndicatorColor = FLinearColor(0.8f, 0.2f, 0.5f, 1.0f);  // 粉色

	/** 体力恢复图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor StaminaIndicatorColor = FLinearColor(0.2f, 0.8f, 0.8f, 1.0f);  // 青色

	/** 默认图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Style")
	FLinearColor DefaultColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);  // 灰色

	/** 中毒效果图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* PoisonIconTexture;

	/** 减速效果图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* SlowIconTexture;

	/** 灼烧效果图标纹理（预留） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* BurnIconTexture;

	/** 攻击Buff图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* AttackBuffIconTexture;

	/** 防御Buff图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* DefenseBuffIconTexture;

	/** 血量恢复效果图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* HealingIndicatorIconTexture;;

	/** 体力恢复效果图标纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Icons")
	UTexture2D* StaminaIndicatorIconTexture;
private:
	/** 当前效果类型 */
	EStatusEffectType EffectType = EStatusEffectType::None;

	/** 总持续时间 */
	float TotalDuration = 0.0f;

	/** 根据效果类型获取对应颜色 */
	FLinearColor GetColorForEffectType(EStatusEffectType Type) const;

	/** 根据效果类型获取对应图标纹理 */
	UTexture2D* GetIconTextureForEffectType(EStatusEffectType Type) const;

	/** 根据效果类型获取对应名称文本 */
	FText GetEffectNameForType(EStatusEffectType Type) const;

	/** 更新图标显示 */
	void UpdateIconDisplay();
};
