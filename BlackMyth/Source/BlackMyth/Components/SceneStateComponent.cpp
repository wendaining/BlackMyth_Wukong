// 场景状态组件 - 管理探索/战斗/Boss战斗状态切换和BGM播放

#include "SceneStateComponent.h"
#include "../EnemyBase.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

USceneStateComponent::USceneStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 默认状态为探索
	CurrentState = ESceneState::Exploration;
}

void USceneStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// 游戏开始时播放探索音乐
	PlayMusicForState(ESceneState::Exploration);
}

void USceneStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CombatCheckTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(CombatEndDelayTimerHandle);
	}

	// 停止音乐
	StopCurrentMusic();

	Super::EndPlay(EndPlayReason);
}

void USceneStateComponent::TransitionToState(ESceneState NewState)
{
	// 如果状态相同则不处理
	if (CurrentState == NewState)
	{
		return;
	}

	ESceneState OldState = CurrentState;
	CurrentState = NewState;

	// 播放对应状态的音乐
	PlayMusicForState(NewState);

	// 根据新状态进行处理
	switch (NewState)
	{
	case ESceneState::NormalCombat:
		// 开始定时检查战斗是否结束
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				CombatCheckTimerHandle,
				this,
				&USceneStateComponent::CheckCombatEnd,
				CombatCheckInterval,
				true // 循环
			);
		}
		break;

	case ESceneState::Exploration:
		// 停止战斗检测计时器
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatCheckTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(CombatEndDelayTimerHandle);
		}
		break;

	case ESceneState::BossCombat:
		// Boss战斗不需要定时检测（由Boss死亡事件触发）
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatCheckTimerHandle);
		}
		break;
	}

	// 广播状态变化
	OnSceneStateChanged.Broadcast(OldState, NewState);

	UE_LOG(LogTemp, Log, TEXT("SceneState: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));
}

void USceneStateComponent::OnCombatInitiated()
{
	// 只有在探索状态时才切换到普通战斗
	if (CurrentState == ESceneState::Exploration)
	{
		// 清除可能存在的延迟切换计时器
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatEndDelayTimerHandle);
		}
		TransitionToState(ESceneState::NormalCombat);
	}
}

void USceneStateComponent::OnBossCombatInitiated()
{
	// 从任何状态都可以切换到Boss战斗（Boss战斗优先级最高）
	if (CurrentState != ESceneState::BossCombat)
	{
		TransitionToState(ESceneState::BossCombat);
	}
}

void USceneStateComponent::OnCombatEnded()
{
	// 从战斗状态切换回探索状态
	if (IsInCombat())
	{
		TransitionToState(ESceneState::Exploration);
	}
}

void USceneStateComponent::PlayMusicForState(ESceneState State)
{
	// 停止当前音乐（带淡出）
	StopCurrentMusic();

	// 选择新音乐
	USoundBase* NewMusic = nullptr;
	switch (State)
	{
	case ESceneState::Exploration:
		NewMusic = ExplorationMusic;
		break;
	case ESceneState::NormalCombat:
		NewMusic = CombatMusic;
		break;
	case ESceneState::BossCombat:
		NewMusic = BossMusic;
		break;
	}

	// 播放新音乐
	if (NewMusic && GetWorld())
	{
		CurrentMusicComponent = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			NewMusic,
			MusicVolume,
			1.0f, // Pitch
			0.0f, // StartTime
			nullptr, // Concurrency
			true, // Persistent
			false  // AutoDestroy
		);

		if (CurrentMusicComponent)
		{
			// 淡入效果
			CurrentMusicComponent->FadeIn(MusicFadeInTime, MusicVolume);

			UE_LOG(LogTemp, Log, TEXT("SceneState: Playing music for state %s"),
				*UEnum::GetValueAsString(State));
		}
	}
}

void USceneStateComponent::StopCurrentMusic()
{
	if (CurrentMusicComponent && CurrentMusicComponent->IsPlaying())
	{
		CurrentMusicComponent->FadeOut(MusicFadeOutTime, 0.0f);
		CurrentMusicComponent = nullptr;
	}
}

void USceneStateComponent::CheckCombatEnd()
{
	// 只在普通战斗状态下检测
	if (CurrentState != ESceneState::NormalCombat)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatCheckTimerHandle);
		}
		return;
	}

	// 检查是否所有敌人都死亡
	if (AreAllEnemiesDeadInRange())
	{
		// 清除检测计时器
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatCheckTimerHandle);

			// 延迟后切换到探索状态（给玩家一些缓冲时间）
			GetWorld()->GetTimerManager().SetTimer(
				CombatEndDelayTimerHandle,
				this,
				&USceneStateComponent::DelayedTransitionToExploration,
				CombatEndDelay,
				false
			);
		}

		UE_LOG(LogTemp, Log, TEXT("SceneState: All enemies dead, will transition to Exploration in %f seconds"), CombatEndDelay);
	}
}

bool USceneStateComponent::AreAllEnemiesDeadInRange()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn)
	{
		return true; // 没有玩家则认为战斗结束
	}

	// 使用球形重叠检测查找范围内的敌人
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerPawn);

	// 检测所有Pawn
	if (GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		PlayerPawn->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(CombatCheckRadius),
		QueryParams))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			// 检查是否为敌人
			if (AEnemyBase* Enemy = Cast<AEnemyBase>(Result.GetActor()))
			{
				// 检查敌人是否存活
				if (UHealthComponent* Health = Enemy->FindComponentByClass<UHealthComponent>())
				{
					if (Health->IsAlive())
					{
						return false; // 还有活着的敌人
					}
				}
			}
		}
	}

	return true; // 所有敌人都死了或没有找到敌人
}

APawn* USceneStateComponent::GetPlayerPawn() const
{
	if (GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}

void USceneStateComponent::DelayedTransitionToExploration()
{
	// 再次检查是否还在战斗状态（可能在延迟期间又遇到敌人）
	if (CurrentState == ESceneState::NormalCombat)
	{
		// 再次确认敌人都死了
		if (AreAllEnemiesDeadInRange())
		{
			TransitionToState(ESceneState::Exploration);
		}
		else
		{
			// 有新敌人，重新开始检测
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(
					CombatCheckTimerHandle,
					this,
					&USceneStateComponent::CheckCombatEnd,
					CombatCheckInterval,
					true
				);
			}
		}
	}
}
