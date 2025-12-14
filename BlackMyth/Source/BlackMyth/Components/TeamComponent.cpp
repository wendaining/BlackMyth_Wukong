// 阵营管理组件实现

#include "TeamComponent.h"

UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// 初始化默认敌对关系
	InitializeDefaultHostility();
}

void UTeamComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTeamComponent::SetTeam(ETeam NewTeam)
{
	if (CurrentTeam != NewTeam)
	{
		ETeam OldTeam = CurrentTeam;
		CurrentTeam = NewTeam;
		OnTeamChanged.Broadcast(OldTeam, NewTeam);
	}
}

bool UTeamComponent::IsHostileTo(ETeam OtherTeam) const
{
	// 中立阵营不主动敌对任何人
	if (CurrentTeam == ETeam::Neutral || OtherTeam == ETeam::Neutral)
	{
		return false;
	}

	// 相同阵营不敌对
	if (CurrentTeam == OtherTeam)
	{
		return false;
	}

	// 检查敌对关系表
	if (const TArray<ETeam>* HostileList = HostileTeams.Find(CurrentTeam))
	{
		return HostileList->Contains(OtherTeam);
	}

	// 默认：不同阵营就敌对（Player vs Enemy）
	return (CurrentTeam == ETeam::Player && OtherTeam == ETeam::Enemy) ||
	       (CurrentTeam == ETeam::Enemy && OtherTeam == ETeam::Player);
}

bool UTeamComponent::IsHostileToActor(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	UTeamComponent* OtherTeamComp = GetTeamComponent(OtherActor);
	if (!OtherTeamComp)
	{
		return false; // 没有阵营组件的 Actor 默认不敌对
	}

	return IsHostileTo(OtherTeamComp->GetTeam());
}

bool UTeamComponent::IsAllyOf(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	UTeamComponent* OtherTeamComp = GetTeamComponent(OtherActor);
	if (!OtherTeamComp)
	{
		return false;
	}

	// 相同阵营是友军
	return CurrentTeam == OtherTeamComp->GetTeam();
}

// ========== 静态辅助函数 ==========

ETeam UTeamComponent::GetActorTeam(AActor* Actor)
{
	if (!Actor)
	{
		return ETeam::Neutral;
	}

	UTeamComponent* TeamComp = GetTeamComponent(Actor);
	return TeamComp ? TeamComp->GetTeam() : ETeam::Neutral;
}

bool UTeamComponent::AreActorsHostile(AActor* ActorA, AActor* ActorB)
{
	if (!ActorA || !ActorB)
	{
		return false;
	}

	UTeamComponent* TeamCompA = GetTeamComponent(ActorA);
	if (!TeamCompA)
	{
		return false;
	}

	return TeamCompA->IsHostileToActor(ActorB);
}

bool UTeamComponent::AreActorsAllies(AActor* ActorA, AActor* ActorB)
{
	if (!ActorA || !ActorB)
	{
		return false;
	}

	UTeamComponent* TeamCompA = GetTeamComponent(ActorA);
	if (!TeamCompA)
	{
		return false;
	}

	return TeamCompA->IsAllyOf(ActorB);
}

UTeamComponent* UTeamComponent::GetTeamComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UTeamComponent>();
}

void UTeamComponent::InitializeDefaultHostility()
{
	// 玩家阵营敌对敌人阵营
	HostileTeams.Add(ETeam::Player, { ETeam::Enemy });
	
	// 敌人阵营敌对玩家阵营
	HostileTeams.Add(ETeam::Enemy, { ETeam::Player });
	
	// 环境对所有人都可能造成伤害（但不是主动敌对）
	// Environment 不在敌对列表中
}
