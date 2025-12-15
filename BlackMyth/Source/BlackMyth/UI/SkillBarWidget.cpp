// 技能栏 Widget 实现

#include "SkillBarWidget.h"
#include "SkillSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void USkillBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化技能栏
	InitializeSkillBar();
}

void USkillBarWidget::InitializeSkillBar()
{
	// 如果技能信息列表为空，使用默认配置
	if (SkillInfoList.Num() == 0)
	{
		// 默认4个技能配置（按照按键顺序）
		SkillInfoList.Add(FSkillInfo(TEXT("分身术"), TEXT("1"), 20.0f));       // 按键1，20秒冷却
		SkillInfoList.Add(FSkillInfo(TEXT("定身术"), TEXT("2"), 15.0f));       // 按键2，15秒冷却
		SkillInfoList.Add(FSkillInfo(TEXT("变身术"), TEXT("3"), 30.0f));       // 按键3，30秒冷却
		SkillInfoList.Add(FSkillInfo(TEXT("法术"), TEXT("4"), 25.0f));         // 按键4，25秒冷却
	}

	// 创建技能槽位
	CreateSkillSlots();
}

void USkillBarWidget::CreateSkillSlots()
{
	if (!SkillSlotsContainer || !SkillSlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillBarWidget: SkillSlotsContainer 或 SkillSlotWidgetClass 未设置"));
		return;
	}

	// 清空现有槽位
	SkillSlotsContainer->ClearChildren();
	SkillSlots.Empty();

	// 创建技能槽位
	int32 NumSlots = FMath::Min(SkillInfoList.Num(), MaxSkillSlots);
	for (int32 i = 0; i < NumSlots; ++i)
	{
		USkillSlotWidget* SlotWidget = CreateWidget<USkillSlotWidget>(this, SkillSlotWidgetClass);
		if (SlotWidget)
		{
			// 初始化槽位
			const FSkillInfo& Info = SkillInfoList[i];
			SlotWidget->InitializeSlot(Info.SkillName, Info.KeyName, Info.Icon);

			// 添加到容器
			UHorizontalBoxSlot* BoxSlot = SkillSlotsContainer->AddChildToHorizontalBox(SlotWidget);
			if (BoxSlot)
			{
				// 设置间距
				BoxSlot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
			}

			SkillSlots.Add(SlotWidget);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SkillBarWidget: 创建了 %d 个技能槽位"), SkillSlots.Num());
}

void USkillBarWidget::TriggerSkillCooldown(int32 SlotIndex, float CooldownDuration)
{
	if (SlotIndex < 0 || SlotIndex >= SkillSlots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillBarWidget: 无效的槽位索引 %d"), SlotIndex);
		return;
	}

	if (USkillSlotWidget* SkillSlot = SkillSlots[SlotIndex])
	{
		SkillSlot->StartCooldown(CooldownDuration);
	}
}

void USkillBarWidget::TriggerSkillCooldownByName(const FString& SkillName, float CooldownDuration)
{
	// 查找技能名称对应的槽位
	for (int32 i = 0; i < SkillInfoList.Num(); ++i)
	{
		if (SkillInfoList[i].SkillName == SkillName)
		{
			TriggerSkillCooldown(i, CooldownDuration);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SkillBarWidget: 未找到技能 %s"), *SkillName);
}

bool USkillBarWidget::IsSkillOnCooldown(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SkillSlots.Num())
	{
		return false;
	}

	if (const USkillSlotWidget* SkillSlot = SkillSlots[SlotIndex])
	{
		return SkillSlot->IsOnCooldown();
	}

	return false;
}

float USkillBarWidget::GetSkillRemainingCooldown(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SkillSlots.Num())
	{
		return 0.0f;
	}

	if (const USkillSlotWidget* SkillSlot = SkillSlots[SlotIndex])
	{
		return SkillSlot->GetRemainingCooldown();
	}

	return 0.0f;
}
