#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HealthComponent.h"
#include "BossHealthBar.generated.h"

class UProgressBar;

/**
 * Boss 血条 UI 类
 */
UCLASS()
class BLACKMYTH_API UBossHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 初始化 Widget，绑定生命值组件 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeWidget(UHealthComponent* NewHealthComponent);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 持有的生命值组件引用 */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** 
	 * 进度条控件
	 * meta = (BindWidget) 表示蓝图中必须有一个同名的 ProgressBar 控件
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;
};
