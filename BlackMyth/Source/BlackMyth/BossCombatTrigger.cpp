// Boss战斗触发器 - 玩家进入区域时触发Boss战斗BGM

#include "BossCombatTrigger.h"
#include "BossEnemy.h"
#include "WukongCharacter.h"
#include "Components/SceneStateComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ABossCombatTrigger::ABossCombatTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABossCombatTrigger::BeginPlay()
{
	Super::BeginPlay();

	// 绑定Boss死亡事件
	BindBossDeathEvent();
}

void ABossCombatTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// 检查是否为玩家
	AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// 如果只触发一次且已触发过，则跳过
	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}

	bHasTriggered = true;

	// 触发Boss战斗状态
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
		{
			SceneComp->OnBossCombatInitiated();
			UE_LOG(LogTemp, Log, TEXT("[BossCombatTrigger] Player entered, initiating Boss combat"));
		}
	}

	// 显示Boss血条
	if (bShowBossHealthOnEnter && LinkedBoss)
	{
		LinkedBoss->SetBossHealthVisibility(true);
	}
}

void ABossCombatTrigger::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	// 检查是否为玩家
	AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// 如果配置为离开时结束战斗
	if (bEndCombatOnLeave)
	{
		if (AGameStateBase* GameState = GetWorld()->GetGameState())
		{
			if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
			{
				SceneComp->OnCombatEnded();
				UE_LOG(LogTemp, Log, TEXT("BossCombatTrigger: Player left, ending Boss combat"));
			}
		}
	}

	// 隐藏Boss血条
	if (bHideBossHealthOnLeave && LinkedBoss)
	{
		LinkedBoss->SetBossHealthVisibility(false);
	}
}

void ABossCombatTrigger::BindBossDeathEvent()
{
	if (LinkedBoss)
	{
		// 获取Boss的HealthComponent并绑定死亡事件
		if (UHealthComponent* BossHealth = LinkedBoss->FindComponentByClass<UHealthComponent>())
		{
			BossHealth->OnDeath.AddDynamic(this, &ABossCombatTrigger::OnBossDeath);
			UE_LOG(LogTemp, Log, TEXT("BossCombatTrigger: Bound to Boss death event"));
		}
	}
}

void ABossCombatTrigger::OnBossDeath(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("BossCombatTrigger: Boss died, ending Boss combat"));

	// Boss死亡时自动切换回探索状态
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
		{
			SceneComp->OnCombatEnded();
		}
	}

	// 隐藏Boss血条
	if (LinkedBoss)
	{
		LinkedBoss->SetBossHealthVisibility(false);
	}
}
