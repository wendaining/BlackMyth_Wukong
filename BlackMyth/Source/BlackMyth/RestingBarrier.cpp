// 安息术屏障实现 - 参考BossCombatTrigger的空气墙方案

#include "RestingBarrier.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "WukongCharacter.h"

ARestingBarrier::ARestingBarrier()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// 创建圆柱形空气墙（使用引擎自带的Cylinder）
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(RootComponent);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 初始关闭碰撞
	WallMesh->SetHiddenInGame(true); // 初始隐藏
	WallMesh->SetCastShadow(false);

	// 加载引擎默认的圆柱体Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		WallMesh->SetStaticMesh(CylinderMesh.Object);
	}

	// 创建地面圆形标记（使用Cylinder压扁）
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(RootComponent);
	FloorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorMesh->SetHiddenInGame(true);
	FloorMesh->SetCastShadow(false);

	if (CylinderMesh.Succeeded())
	{
		FloorMesh->SetStaticMesh(CylinderMesh.Object);
	}

	// 创建特效组件
	BarrierEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BarrierEffect"));
	BarrierEffect->SetupAttachment(RootComponent);
	BarrierEffect->bAutoActivate = false;
}

void ARestingBarrier::BeginPlay()
{
	Super::BeginPlay();

	// 播放生成音效
	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}

	// 如果有特效资产，启动特效
	if (BarrierEffectAsset && BarrierEffect)
	{
		BarrierEffect->SetAsset(BarrierEffectAsset);
		BarrierEffect->Activate(true);
	}

	UE_LOG(LogTemp, Warning, TEXT("[RestingBarrier] BeginPlay at %s"), *GetActorLocation().ToString());
}

void ARestingBarrier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 只有屏障激活时才约束悟空
	if (bIsActive)
	{
		ConstrainCasterToCircle();
	}
}

void ARestingBarrier::InitializeBarrier(float InDuration)
{
	BarrierDuration = InDuration;

	// 记录屏障中心位置
	BarrierCenter = GetActorLocation();

	// 获取施法者（悟空）
	CasterCharacter = Cast<AWukongCharacter>(GetOwner());

	// 激活屏障
	SetBarrierActive(true);

	UE_LOG(LogTemp, Warning, TEXT("[RestingBarrier] Initialized - Center: %s, Radius: %.1f, Duration: %.1fs"), 
		*BarrierCenter.ToString(), BarrierRadius, BarrierDuration);

	// 设置自动销毁定时器
	GetWorldTimerManager().SetTimer(CleanupTimer, this, &ARestingBarrier::CleanupBarrier, BarrierDuration, false);
}

void ARestingBarrier::SetBarrierActive(bool bActive)
{
	bIsActive = bActive;

	if (bActive)
	{
		// 更新墙体尺寸
		UpdateWallScale();

		// 显示墙体并开启碰撞
		if (WallMesh)
		{
			if (WallMaterial)
			{
				WallMesh->SetMaterial(0, WallMaterial);
			}
			WallMesh->SetHiddenInGame(false);
			WallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			
			// 设置碰撞对象类型为 WorldDynamic
			WallMesh->SetCollisionObjectType(ECC_WorldDynamic);
			
			// 阻挡 Pawn（敌人）和其他物理物体，但忽略摄像机和可见性
			WallMesh->SetCollisionResponseToAllChannels(ECR_Block);
			WallMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);  // 不挡视线/摄像机
			WallMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);      // 不挡摄像机
			
			// 关键：让悟空的胶囊体忽略这个墙体
			if (CasterCharacter)
			{
				// 双向忽略
				WallMesh->MoveIgnoreActors.Add(CasterCharacter);
				CasterCharacter->MoveIgnoreActorAdd(this);
				
				// 让悟空的碰撞组件也忽略墙体
				if (UCapsuleComponent* CasterCapsule = CasterCharacter->GetCapsuleComponent())
				{
					CasterCapsule->MoveIgnoreActors.Add(this);
				}
			}
		}

		// 显示地面圆形
		if (FloorMesh)
		{
			if (FloorMaterial)
			{
				FloorMesh->SetMaterial(0, FloorMaterial);
			}
			FloorMesh->SetHiddenInGame(false);
		}

		UE_LOG(LogTemp, Warning, TEXT("[RestingBarrier] Barrier ACTIVATED - Radius: %.1f, Height: %.1f"), 
			BarrierRadius, BarrierHeight);
	}
	else
	{
		// 恢复悟空的碰撞忽略列表
		if (CasterCharacter)
		{
			CasterCharacter->MoveIgnoreActorRemove(this);
			
			// 恢复悟空碰撞组件的忽略列表
			if (UCapsuleComponent* CasterCapsule = CasterCharacter->GetCapsuleComponent())
			{
				CasterCapsule->MoveIgnoreActors.Remove(this);
			}
		}
		
		// 隐藏墙体并关闭碰撞
		if (WallMesh)
		{
			WallMesh->SetHiddenInGame(true);
			WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WallMesh->MoveIgnoreActors.Empty();
		}

		if (FloorMesh)
		{
			FloorMesh->SetHiddenInGame(true);
		}

		UE_LOG(LogTemp, Warning, TEXT("[RestingBarrier] Barrier DEACTIVATED"));
	}
}

void ARestingBarrier::UpdateWallScale()
{
	// 引擎默认Cylinder的尺寸是半径50, 高度100 (从-50到+50)
	// 我们需要将其缩放到目标尺寸
	const float BaseCylinderRadius = 50.0f;
	const float BaseCylinderHeight = 100.0f;

	// 计算墙体缩放
	float RadiusScale = BarrierRadius / BaseCylinderRadius;
	float HeightScale = BarrierHeight / BaseCylinderHeight;

	if (WallMesh)
	{
		WallMesh->SetRelativeScale3D(FVector(RadiusScale, RadiusScale, HeightScale));
		// 将墙体抬高，使底部在地面
		WallMesh->SetRelativeLocation(FVector(0.0f, 0.0f, BarrierHeight / 2.0f));
	}

	// 地面圆形：压扁的圆柱
	if (FloorMesh)
	{
		float FloorRadiusScale = BarrierRadius / BaseCylinderRadius;
		FloorMesh->SetRelativeScale3D(FVector(FloorRadiusScale, FloorRadiusScale, 0.02f)); // 非常薄
		FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 1.0f)); // 稍微抬高避免Z-fighting
	}
}

void ARestingBarrier::SetBarrierRadius(float NewRadius)
{
	if (NewRadius <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RestingBarrier] Invalid radius: %.1f, must be > 0"), NewRadius);
		return;
	}

	BarrierRadius = NewRadius;

	// 如果屏障已激活，更新尺寸
	if (bIsActive)
	{
		UpdateWallScale();
	}

	UE_LOG(LogTemp, Log, TEXT("[RestingBarrier] Radius updated to %.1f"), BarrierRadius);
}

void ARestingBarrier::ConstrainCasterToCircle()
{
	if (!CasterCharacter) return;

	// 计算悟空到屏障中心的2D距离（忽略Z轴）
	FVector CasterLocation = CasterCharacter->GetActorLocation();
	float DistanceToCenter = FVector::Dist2D(CasterLocation, BarrierCenter);

	// 留一点缓冲区（比墙体半径小一点）
	float EffectiveRadius = BarrierRadius - 30.0f;

	// 如果超出有效半径，将悟空拉回
	if (DistanceToCenter > EffectiveRadius)
	{
		// 计算从中心指向悟空的方向（2D）
		FVector Direction = (CasterLocation - BarrierCenter).GetSafeNormal2D();
		
		// 计算边界位置
		FVector BoundaryPosition = BarrierCenter + Direction * (EffectiveRadius - 10.0f);
		BoundaryPosition.Z = CasterLocation.Z; // 保持原Z轴高度
		
		// 设置悟空位置
		CasterCharacter->SetActorLocation(BoundaryPosition);
		
		// 取消超出方向的速度
		if (UCharacterMovementComponent* MovementComp = CasterCharacter->GetCharacterMovement())
		{
			FVector Velocity = MovementComp->Velocity;
			float DirectionalSpeed = FVector::DotProduct(Velocity, Direction);
			if (DirectionalSpeed > 0)
			{
				Velocity -= Direction * DirectionalSpeed;
				MovementComp->Velocity = Velocity;
			}
		}
	}
}

void ARestingBarrier::CleanupBarrier()
{
	UE_LOG(LogTemp, Log, TEXT("[RestingBarrier] Cleaning up barrier"));

	// 关闭屏障
	SetBarrierActive(false);

	// 播放消失音效
	if (DespawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DespawnSound, GetActorLocation());
	}

	// 停止特效
	if (BarrierEffect)
	{
		BarrierEffect->Deactivate();
	}

	// 清空施法者引用
	CasterCharacter = nullptr;

	// 延迟销毁
	SetLifeSpan(0.5f);
}
