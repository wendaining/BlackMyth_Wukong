// 技能栏 Widget - 包含4个技能槽位
// 显示在屏幕右下角

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillBarWidget.generated.h"

class USkillSlotWidget;
class UHorizontalBox;

// 技能信息结构体
USTRUCT(BlueprintType)
struct FSkillInfo
{
	GENERATED_BODY()

	// 技能名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkillName;

	// 对应的按键
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyName;

	// 技能图标
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	// 冷却时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 10.0f;

	FSkillInfo() {}
	FSkillInfo(const FString& InName, const FString& InKey, float InCooldown)
		: SkillName(InName), KeyName(InKey), CooldownTime(InCooldown) {}
};

/**
 * 技能栏 Widget
 * 管理4个技能槽位，显示技能图标和冷却状态
 */
UCLASS()
class BLACKMYTH_API USkillBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 初始化技能栏
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void InitializeSkillBar();

	// 触发技能冷却（按槽位索引 0-3）
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void TriggerSkillCooldown(int32 SlotIndex, float CooldownDuration);

	// 触发技能冷却（按技能名称）
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void TriggerSkillCooldownByName(const FString& SkillName, float CooldownDuration);

	// 检查技能是否在冷却中
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool IsSkillOnCooldown(int32 SlotIndex) const;

	// 获取技能剩余冷却时间
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillRemainingCooldown(int32 SlotIndex) const;

protected:
	virtual void NativeConstruct() override;

	// 创建技能槽位
	void CreateSkillSlots();

protected:
	// 技能槽位容器
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* SkillSlotsContainer;

	// 技能槽位 Widget 类（用于动态创建）
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<USkillSlotWidget> SkillSlotWidgetClass;

	// 技能信息列表（4个技能）
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TArray<FSkillInfo> SkillInfoList;

	// 技能槽位实例
	UPROPERTY()
	TArray<USkillSlotWidget*> SkillSlots;

	// 最大技能数量
	static constexpr int32 MaxSkillSlots = 4;
};
