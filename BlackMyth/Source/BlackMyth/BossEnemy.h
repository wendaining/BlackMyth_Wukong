#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy.generated.h"

class UBossHealthBar;

/**
 * Boss 敌人
 */
UCLASS()
class BLACKMYTH_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	ABossEnemy();

	virtual void BeginPlay() override;

	/** 设置 Boss 血条可见性 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBossHealthVisibility(bool bVisible);

protected:
	/** Boss 血条 UI 类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBossHealthBar> BossHealthBarClass;

	/** Boss 血条 UI 实例 */
	UPROPERTY()
	UBossHealthBar* BossHealthBarWidget;
};
