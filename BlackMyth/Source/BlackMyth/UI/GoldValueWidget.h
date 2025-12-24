// 金币价值显示Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoldValueWidget.generated.h"

class UTextBlock;

/**
 * 金币价值显示Widget
 * 显示金币的数值（如 "12"）
 */
UCLASS()
class BLACKMYTH_API UGoldValueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 设置显示的金币数值 */
	UFUNCTION(BlueprintCallable, Category = "Gold")
	void SetGoldValue(int32 Value);

protected:
	/** 金币数值文本（需在蓝图中绑定） */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueText;

	virtual void NativeConstruct() override;
};
