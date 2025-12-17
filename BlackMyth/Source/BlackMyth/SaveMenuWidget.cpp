#include "SaveMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BlackMythSaveGame.h"
#include "WukongCharacter.h"
#include "EnemySpawner.h"
#include "EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Components/StaminaComponent.h"

void USaveMenuWidget::OnSaveSlotClicked(int32 SlotIndex) {
	if (SlotIndex < 1) {
		return;
	}

	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!Player) {
		return;
	}

	// 创建或覆盖存档
	UBlackMythSaveGame* SaveGame = Cast<UBlackMythSaveGame>(
		UGameplayStatics::CreateSaveGameObject(
			UBlackMythSaveGame::StaticClass()));

	if (!SaveGame) {
		return;
	}

	// 保存玩家状态
	SaveGame->PlayerLocation = Player->GetActorLocation();
	SaveGame->PlayerRotation = Player->GetActorRotation();

	// 保存主角血量和体力
	if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(Player)) {
		if (UHealthComponent* HealthComp = Wukong->GetHealthComponent()) {
			SaveGame->PlayerHealth = HealthComp->GetCurrentHealth();
			SaveGame->PlayerMaxHealth = HealthComp->GetMaxHealth();
		}
		if (UStaminaComponent* StaminaComp = Wukong->GetStaminaComponent()) {
			SaveGame->PlayerStamina = StaminaComp->GetCurrentStamina();
			SaveGame->PlayerMaxStamina = StaminaComp->GetMaxStamina();
		}
	}

	// 保存所有敌人
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemySpawner::StaticClass(), FoundSpawners);

	SaveGame->Enemies.Empty();

	// 遍历所有EnemySpawner，保存它们生成的所有敌人
	for (AActor* SpawnerActor : FoundSpawners)
	{
		AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor);
		if (Spawner)
		{
			for (AEnemyBase* Enemy : Spawner->SpawnedEnemies)
			{
				if (IsValid(Enemy))
				{
					FEnemySaveData Data;
					Enemy->WriteEnemySaveData(Data);
					SaveGame->Enemies.Add(Data);
				}
			}
		}
	}
	// 存档名（来自输入框）
	if (SaveNameTextBox && !SaveNameTextBox->GetText().IsEmpty()) {
		SaveGame->SaveName = SaveNameTextBox->GetText().ToString();
	}
	else {
		SaveGame->SaveName = FString::Printf(TEXT("Save Slot %d"), SlotIndex);
	}

	// 记录存档时间（给 LoadMenu 显示备用）
	SaveGame->SaveTime = FDateTime::Now();

	const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);

	RemoveFromParent();
}

void USaveMenuWidget::NativeConstruct() {
	Super::NativeConstruct();

	UpdateSlotInfo(1, SaveSlot1Text);
	UpdateSlotInfo(2, SaveSlot2Text);
	UpdateSlotInfo(3, SaveSlot3Text);
}

void USaveMenuWidget::UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text) {
	if (!Text) return;

	const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0)) {
		UBlackMythSaveGame* SaveGame = Cast<UBlackMythSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (SaveGame) {
			Text->SetText(FText::FromString(SaveGame->SaveName));
			return;
		}
	}

	Text->SetText(FText::FromString(TEXT("空存档")));
}
