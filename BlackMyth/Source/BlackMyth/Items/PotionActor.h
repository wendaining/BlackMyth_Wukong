// 药瓶Actor父类 - 用于喝药动画中显示的药瓶模型
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PotionActor.generated.h"

/**
 * 药瓶Actor父类
 * 用于在喝药动画中显示的药瓶模型
 */
UCLASS()
class BLACKMYTH_API APotionActor : public AActor
{
	GENERATED_BODY()

public:
	APotionActor();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 组件 ==========

	/** 根组件 - 场景组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** 药瓶网格体 - StaticMesh（在蓝图中配置具体模型） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PotionMesh;

	// ========== 可配置属性 ==========

	/** 药瓶类型名称（用于调试） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	FString PotionTypeName = TEXT("Generic Potion");

	/** 附加到的骨骼名称（默认为右手）*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion")
	FName AttachSocketName = TEXT("hand_rSocket");

	// ========== 功能函数 ==========

	/** 显示药瓶 */
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void ShowPotion();

	/** 隐藏药瓶 */
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void HidePotion();

	/** 附加到角色的骨骼 */
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void AttachToCharacter(ACharacter* Character);

	/** 从角色分离并销毁 */
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void DetachAndDestroy();
};
