// 立棍法AOE伤害通知实现

#include "AnimNotify_PoleStanceAOE.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "../EnemyBase.h"

UAnimNotify_PoleStanceAOE::UAnimNotify_PoleStanceAOE()
{
	AOERadius = 400.0f;
	Damage = 60.0f;
	bDrawDebug = true;
	DebugDrawDuration = 3.0f;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 128, 0); // 橙色
#endif
}

void UAnimNotify_PoleStanceAOE::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 明确的触发日志
	UE_LOG(LogTemp, Error, TEXT("========== [PoleStanceAOE] NOTIFY TRIGGERED! =========="));

	if (!MeshComp || !MeshComp->GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[PoleStanceAOE] ERROR: Invalid MeshComp or World!"));
		return;
	}

	// 获取角色
	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("[PoleStanceAOE] ERROR: Owner is not a Character!"));
		return;
	}

	// 获取角色位置作为AOE中心点
	FVector AOECenter = OwnerCharacter->GetActorLocation();
	UWorld* World = MeshComp->GetWorld();

	UE_LOG(LogTemp, Warning, TEXT("[PoleStanceAOE] Center: %s, Radius: %.1f, Damage: %.1f"), 
		*AOECenter.ToString(), AOERadius, Damage);

	// 绘制调试球体（更明显）
	if (bDrawDebug)
	{
		DrawDebugSphere(World, AOECenter, AOERadius, 32, FColor::Orange, false, DebugDrawDuration, 0, 5.0f);
		
		// 屏幕调试信息
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
				FString::Printf(TEXT("立棍AOE触发! 半径:%.0f 伤害:%.0f"), AOERadius, Damage));
		}
	}

	// 球形范围检测（使用OverlapMulti更可靠）
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(AOERadius);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;

	// 使用 OverlapMultiByChannel 检测所有Pawn
	bool bHit = World->OverlapMultiByChannel(
		OverlapResults,
		AOECenter,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);

	UE_LOG(LogTemp, Warning, TEXT("[PoleStanceAOE] Overlap check returned: %d results"), OverlapResults.Num());

	if (!bHit || OverlapResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PoleStanceAOE] No targets found in radius %.1f"), AOERadius);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("立棍AOE: 范围内无敌人"));
		}
		return;
	}

	// 统计命中敌人数量
	int32 EnemyHitCount = 0;

	// 遍历所有命中对象
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor)
		{
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("[PoleStanceAOE] Found actor: %s"), *HitActor->GetName());

		// 只处理敌人
		AEnemyBase* Enemy = Cast<AEnemyBase>(HitActor);
		if (!Enemy)
		{
			UE_LOG(LogTemp, Log, TEXT("[PoleStanceAOE]   -> Not an enemy, skipping"));
			continue;
		}

		if (Enemy->IsDead())
		{
			UE_LOG(LogTemp, Log, TEXT("[PoleStanceAOE]   -> Enemy is dead, skipping"));
			continue;
		}

		// 立棍作为AOE技能，直接造成伤害，无视闪避
		// 伤害会触发敌人正常的受击硬直动画，产生自然的控制效果
		Enemy->ReceiveDamage(Damage, OwnerCharacter, false); // false = 不允许闪避

		EnemyHitCount++;

		// 绘制命中线
		if (bDrawDebug)
		{
			DrawDebugLine(World, AOECenter, Enemy->GetActorLocation(), FColor::Red, false, DebugDrawDuration, 0, 5.0f);
			DrawDebugSphere(World, Enemy->GetActorLocation(), 50.0f, 12, FColor::Red, false, DebugDrawDuration, 0, 3.0f);
		}

		UE_LOG(LogTemp, Error, TEXT("[PoleStanceAOE] >>> HIT ENEMY: %s, Damage: %.1f"), 
			*Enemy->GetName(), Damage);
	}

	UE_LOG(LogTemp, Error, TEXT("[PoleStanceAOE] ===== TOTAL ENEMIES HIT: %d ====="), EnemyHitCount);

	if (GEngine && EnemyHitCount > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			FString::Printf(TEXT("立棍AOE命中 %d 个敌人!"), EnemyHitCount));
	}
}

#if WITH_EDITOR
FString UAnimNotify_PoleStanceAOE::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("立棍AOE (R:%.0f D:%.0f)"), AOERadius, Damage);
}
#endif
