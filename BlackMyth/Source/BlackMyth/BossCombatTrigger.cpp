#include "BossCombatTrigger.h"
#include "BossEnemy.h"
#include "WukongCharacter.h"
#include "Components/SceneStateComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dialogue/DialogueComponent.h"
#include "Dialogue/DialogueData.h"
#include "XiaoTian.h"
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

	// 初始化对话组件
	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));
}

void ABossCombatTrigger::BeginPlay()
{
	Super::BeginPlay();

	// 绑定Boss死亡事件
	BindBossDeathEvent();

	// 绑定对话结束事件
	if (DialogueComponent)
	{
		DialogueComponent->OnDialogueStateChanged.AddDynamic(this, &ABossCombatTrigger::OnCutsceneFinished);
		DialogueComponent->OnDialogueEvent.AddDynamic(this, &ABossCombatTrigger::HandleDialogueEvent);
		DialogueComponent->OnCameraTargetChanged.AddDynamic(this, &ABossCombatTrigger::HandleCameraTargetChanged);
	}
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

	// [New] 延迟开始过场动画，给玩家一点走位时间进入中心，避免卡在门口
	UE_LOG(LogTemp, Warning, TEXT("[BossArea] Player entered. Delaying CG for %.1f seconds..."), CutsceneStartDelay);
	
	FTimerHandle StartTimer;
	GetWorldTimerManager().SetTimer(StartTimer, [this, Player]() {
		this->StartBossCutscene(Player);
	}, CutsceneStartDelay, false);
}

void ABossCombatTrigger::StartBossCutscene(APawn* PlayerPawn)
{
	if (!PlayerPawn || !LinkedBoss || !DialogueComponent)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
	if (PC)
	{
		// 1. 设置说话人为 Boss (这样 Boss 才会做动作)
		DialogueComponent->AlternativeSpeaker = LinkedBoss;

		// 2. 切换镜头到 Boss
		PC->SetViewTargetWithBlend(LinkedBoss, CameraBlendTime);
		
		// 3. 开始对话 (DialogueComponent 会自动处理玩家输入锁定)
		DialogueComponent->StartDialogue();
		
		UE_LOG(LogTemp, Log, TEXT("[BossCombatTrigger] Starting Cutscene... Camera blending to Boss."));
	}
}

void ABossCombatTrigger::OnCutsceneFinished(bool bIsPlaying)
{
	if (bIsPlaying) return; // 只处理“结束”消息

	UE_LOG(LogTemp, Log, TEXT("[BossCombatTrigger] Cutscene finished. Starting combat logic."));

	// 1. 切换镜头回玩家
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->SetViewTargetWithBlend(PC->GetPawn(), CameraBlendTime);
	}

	// 2. 原有的战斗启动逻辑
	AWukongCharacter* Player = Cast<AWukongCharacter>(PC ? PC->GetPawn() : nullptr);
	
	// 触发Boss战斗状态
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
		{
			SceneComp->OnBossCombatInitiated();
		}
	}

	// 激活 Boss
	if (LinkedBoss && Player)
	{
		LinkedBoss->ActivateBoss(Player);
	}

	// 延迟2秒开启结界
	FTimerHandle BarrierTimer;
	GetWorldTimerManager().SetTimer(BarrierTimer, [this, Player]()
	{
		if (IsValid(Player) && IsOverlappingActor(Player))
		{
			SetBarrierActive(true);
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

void ABossCombatTrigger::HandleDialogueEvent(const FString& EventTag)
{
	if (EventTag == TEXT("SummonDog") && LinkedBoss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cutscene] SummonDog Event Received!"));

		// 1. 获取生成位置 (基于 Boss 的位置和偏移)
		FVector SpawnLocation = LinkedBoss->GetActorLocation() + LinkedBoss->GetActorRotation().RotateVector(LinkedBoss->DogSpawnOffset);
		FRotator SpawnRotation = LinkedBoss->GetActorRotation();

		// 2. 生成哮天犬
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = LinkedBoss.Get();
		SpawnParams.Instigator = LinkedBoss.Get();

		if (AActor* DogActor = GetWorld()->SpawnActor<AActor>(LinkedBoss->DogClass, SpawnLocation, SpawnRotation, SpawnParams))
		{
			// 3. 通知哮天犬播放 End 动作并消失 (即过场动画模式)
			if (AXiaoTian* Dog = Cast<AXiaoTian>(DogActor))
			{
				Dog->PlayEndAndVanish();
			}
		}
	}
}

void ABossCombatTrigger::HandleCameraTargetChanged(AActor* NewTarget)
{
	if (!NewTarget) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cutscene] Camera Blending to: %s"), *NewTarget->GetName());
		PC->SetViewTargetWithBlend(NewTarget, CameraBlendTime);
	}
}
