// 金币组件 - 管理玩家的金币

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WalletComponent.generated.h"

// 委托声明：金币变化时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UWalletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWalletComponent();

	// === 金币属性 ===

	/** 当前金币数量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wallet")
	int32 CurrentGold = 0;

	// === 核心接口 ===

	/** 获取当前金币 */
	UFUNCTION(BlueprintPure, Category = "Wallet")
	int32 GetGold() const { return CurrentGold; }

	/** 增加金币 */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	void AddGold(int32 Amount);

	/** 消费金币（返回是否成功） */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	bool SpendGold(int32 Amount);

	/** 检查是否有足够金币 */
	UFUNCTION(BlueprintPure, Category = "Wallet")
	bool HasEnoughGold(int32 Amount) const { return CurrentGold >= Amount; }

	/** 设置金币数量（用于读档） */
	UFUNCTION(BlueprintCallable, Category = "Wallet")
	void SetGold(int32 Amount);

	// === 委托 ===

	/** 金币数量变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Wallet|Events")
	FOnGoldChanged OnGoldChanged;

protected:
	virtual void BeginPlay() override;
};
