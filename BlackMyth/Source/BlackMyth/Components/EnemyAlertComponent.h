#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAlertComponent.generated.h"

class UWidgetComponent;
class UUserWidget;
class AEnemyBase;

// 敌人警戒组件 - 实现协同警戒系统
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UEnemyAlertComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAlertComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 广播警报给周围敌人 */
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void BroadcastAlert(AActor* Target, float AlertRadius);

	/** 接收来自其他敌人的警报 */
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void ReceiveAlert(AActor* AlertTarget);

	/** 显示/隐藏警戒图标 */
	UFUNCTION(BlueprintCallable, Category = "Alert")
	void ShowAlertIcon(bool bShow);

	/** 是否处于警戒状态 */
	UFUNCTION(BlueprintPure, Category = "Alert")
	bool IsAlerted() const { return bIsAlerted; }

	/** 获取当前警戒目标 */
	UFUNCTION(BlueprintPure, Category = "Alert")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

protected:
	/** 警戒图标 Widget 类 */
	UPROPERTY(EditDefaultsOnly, Category = "Alert|UI")
	TSubclassOf<UUserWidget> AlertIconWidgetClass;

	/** 警报音效 */
	UPROPERTY(EditDefaultsOnly, Category = "Alert|Audio")
	TObjectPtr<USoundBase> AlertSound;

	/** 警戒图标高度偏移（相对于敌人头顶） */
	UPROPERTY(EditDefaultsOnly, Category = "Alert|UI")
	float AlertIconHeightOffset = 150.0f;

	/** 默认警报半径（单位：cm） */
	UPROPERTY(EditDefaultsOnly, Category = "Alert")
	float DefaultAlertRadius = 1000.0f;

	/** 是否自动隐藏警戒图标 */
	UPROPERTY(EditDefaultsOnly, Category = "Alert|UI")
	bool bAutoHideIcon = true;

	/** 自动隐藏延迟（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Alert|UI", meta = (EditCondition = "bAutoHideIcon"))
	float AutoHideDelay = 5.0f;

private:
	/** 缓存的敌人引用 */
	UPROPERTY()
	TObjectPtr<AEnemyBase> OwnerEnemy;

	/** 警戒图标 Widget 组件 */
	UPROPERTY()
	TObjectPtr<UWidgetComponent> AlertIconWidget;

	/** 是否处于警戒状态 */
	bool bIsAlerted;

	/** 当前警戒目标 */
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	/** 自动隐藏计时器 */
	FTimerHandle AutoHideTimer;

	/** 自动隐藏警戒图标回调 */
	void AutoHideAlertIcon();
};
