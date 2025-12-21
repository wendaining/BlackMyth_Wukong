#include "BossCombatTrigger.h"
#include "BossEnemy.h"
#include "WukongCharacter.h"
#include "Components/SceneStateComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABossCombatTrigger::ABossCombatTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建视觉外壳 (四面墙)
	auto CreateWall = [this](TObjectPtr<UStaticMeshComponent>& Wall, FName Name) {
		Wall = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Wall->SetupAttachment(GetBrushComponent());
		Wall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Wall->SetHiddenInGame(true);
		Wall->SetCastShadow(false);
	};

	CreateWall(WallN, TEXT("WallN"));
	CreateWall(WallS, TEXT("WallS"));
	CreateWall(WallE, TEXT("WallE"));
	CreateWall(WallW, TEXT("WallW"));

	// 加载引擎默认的 Cube 作为墙体形状
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		WallN->SetStaticMesh(CubeMeshAsset.Object);
		WallS->SetStaticMesh(CubeMeshAsset.Object);
		WallE->SetStaticMesh(CubeMeshAsset.Object);
		WallW->SetStaticMesh(CubeMeshAsset.Object);
	}
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

	// 激活 Boss
	if (LinkedBoss)
	{
		LinkedBoss->ActivateBoss(Player);
	}

	// [New] 延迟2秒开启结界，并确保玩家已经在区域内 (防止把玩家关在外面)
	FTimerHandle BarrierTimer;
	GetWorldTimerManager().SetTimer(BarrierTimer, [this, Player]()
	{
		// 只有当玩家仍然在触发器范围内时，才升起结界
		if (IsValid(Player) && IsOverlappingActor(Player))
		{
			SetBarrierActive(true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[BossArea] Player is NOT in area after 2s. Barrier canceled to prevent locking out."));
			// 注意：Boss可能已经激活了，这里只取消结界
		}
	}, 2.0f, false);
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

	// [New] 关闭战斗边界
	SetBarrierActive(false);
}

void ABossCombatTrigger::SetBarrierActive(bool bActive)
{
	UBrushComponent* BrushComp = GetBrushComponent();
	if (!BrushComp) return;

	TArray<UStaticMeshComponent*> WallArray = { WallN, WallS, WallE, WallW };

	if (bActive)
	{
		// 1. 物理阻挡：不再把整个 Volume 变硬 (否则会卡在里面)
		// 仍然保持 Trigger 模式，让 4 面墙负责阻挡
		
		// 2. 视觉显现 + 物理墙：根据 Volume 的大小自动适配 4 面墙的位置和缩放
		if (BarrierMaterial)
		{
			// 计算 Volume 的包围盒
			FBox LocalBox = BrushComp->CalcBounds(FTransform::Identity).GetBox();
			FVector Extent = LocalBox.GetExtent();
			
			// 墙的厚度
			const float Thickness = 10.0f;
			const float ThicknessScale = Thickness / 50.0f; // Cube 默认是 50 半径

			// 适配高度和长度
			float ScaleX = Extent.X / 50.0f;
			float ScaleY = Extent.Y / 50.0f;
			float ScaleZ = Extent.Z / 50.0f;

			// 设置 N (前)
			WallN->SetRelativeLocation(FVector(Extent.X, 0.f, 0.f));
			WallN->SetRelativeScale3D(FVector(ThicknessScale, ScaleY, ScaleZ));

			// 设置 S (后)
			WallS->SetRelativeLocation(FVector(-Extent.X, 0.f, 0.f));
			WallS->SetRelativeScale3D(FVector(ThicknessScale, ScaleY, ScaleZ));

			// 设置 E (右)
			WallE->SetRelativeLocation(FVector(0.f, Extent.Y, 0.f));
			WallE->SetRelativeScale3D(FVector(ScaleX, ThicknessScale, ScaleZ));

			// 设置 W (左)
			WallW->SetRelativeLocation(FVector(0.f, -Extent.Y, 0.f));
			WallW->SetRelativeScale3D(FVector(ScaleX, ThicknessScale, ScaleZ));

			for (UStaticMeshComponent* Wall : WallArray)
			{
				if (Wall)
				{
					Wall->SetMaterial(0, BarrierMaterial);
					Wall->SetHiddenInGame(false);
					Wall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					Wall->SetCollisionResponseToAllChannels(ECR_Block);
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[BossArea] Hollow Arena Barrier ACTIVATED. 4 Walls engaged."));
	}
	else
	{
		// 关闭视觉和碰撞
		for (UStaticMeshComponent* Wall : WallArray)
		{
			if (Wall)
			{
				Wall->SetHiddenInGame(true);
				Wall->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[BossArea] Arena Barrier DEACTIVATED."));
	}
}
