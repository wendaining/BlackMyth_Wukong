// 技能槽位 Widget - 显示单个技能图标和冷却进度

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;

/**
 * 单个技能槽位 Widget
 * 显示技能图标、按键提示和冷却进度
 */
UCLASS()
class BLACKMYTH_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 初始化技能槽位 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void InitializeSlot(const FString& InSkillName, const FString& InKeyName, UTexture2D* InIcon = nullptr);

	/** 开始冷却 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void StartCooldown(float CooldownDuration);

	/** 更新冷却进度（每帧调用） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void UpdateCooldown(float DeltaTime);

	/** 重置冷却（立即可用） */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ResetCooldown();

	/** 获取技能名称 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	FString GetSkillName() const { return SkillName; }

	/** 是否正在冷却中 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsOnCooldown() const { return bIsOnCooldown; }

	/** 获取剩余冷却时间 */
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetRemainingCooldown() const { return RemainingCooldown; }

	/** 获取冷却进度 (0-1, 0=刚开始冷却, 1=冷却完成) */
	UFUNCTION(BlueprintPure, Category = "Skill")
	float GetCooldownProgress() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ========== UI 控件绑定 ==========

	/** 技能图标背景（圆盘） */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* SkillBackground;

	/** 技能图标 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* SkillIcon;

	/** 冷却遮罩（灰色覆盖层） */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CooldownOverlay;

	/** 按键提示文本 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* KeyText;

	/** 冷却时间文本 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CooldownText;

	/** 冷却进度条 */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* CooldownBar;

	// ========== 配置属性 ==========

	/** 可用状态时的颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor AvailableColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);  // 金色

	/** 冷却中的颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor CooldownColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f);  // 灰色

	/** 冷却进度条颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor CooldownBarColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);  // 蓝色

private:
	/** 技能名称（用于匹配冷却） */
	FString SkillName;

	/** 按键名称 */
	FString KeyName;

	/** 是否正在冷却 */
	bool bIsOnCooldown = false;

	/** 总冷却时间 */
	float TotalCooldown = 0.0f;

	/** 剩余冷却时间 */
	float RemainingCooldown = 0.0f;

	/** 更新视觉效果 */
	void UpdateVisuals();
};
