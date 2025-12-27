// 金币组件实现

#include "WalletComponent.h"

UWalletComponent::UWalletComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentGold = 100;
}

void UWalletComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWalletComponent::AddGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentGold += Amount;

	// 广播金币变化事件
	OnGoldChanged.Broadcast(CurrentGold);

	UE_LOG(LogTemp, Log, TEXT("[WalletComponent] Added %d gold. Current: %d"), Amount, CurrentGold);
}

bool UWalletComponent::SpendGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	if (!HasEnoughGold(Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WalletComponent] Not enough gold. Required: %d, Current: %d"), Amount, CurrentGold);
		return false;
	}

	CurrentGold -= Amount;

	// 广播金币变化事件
	OnGoldChanged.Broadcast(CurrentGold);

	UE_LOG(LogTemp, Log, TEXT("[WalletComponent] Spent %d gold. Current: %d"), Amount, CurrentGold);
	return true;
}

void UWalletComponent::SetGold(int32 Amount)
{
	CurrentGold = FMath::Max(0, Amount);
	OnGoldChanged.Broadcast(CurrentGold);

	UE_LOG(LogTemp, Log, TEXT("[WalletComponent] Gold set to %d"), CurrentGold);
}
