// 金币掉落物

#include "GoldPickup.h"
#include "../WukongCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Components/WalletComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AGoldPickup::AGoldPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootSceneComponent);

	// 创建金币网格体
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootSceneComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 默认使用引擎自带的球体作为占位符（实际项目中应在蓝图中设置金币模型）
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (DefaultMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(DefaultMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	}

	// 设置金色材质
	static ConstructorHelpers::FObjectFinder<UMaterial> GoldMaterial(
		TEXT("/Engine/EngineMaterials/DefaultMaterial"));
	if (GoldMaterial.Succeeded())
	{
		MeshComponent->SetMaterial(0, GoldMaterial.Object);
	}

	// 创建碰撞球体
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootSceneComponent);
	CollisionSphere->SetSphereRadius(AutoPickupRadius);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
}

void AGoldPickup::BeginPlay()
{
	Super::BeginPlay();

	// 绑定重叠事件
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AGoldPickup::OnPlayerEnterRange);
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AGoldPickup::OnPlayerExitRange);
		// 更新碰撞半径（使用较大的吸附半径）
		CollisionSphere->SetSphereRadius(AttractRadius);
	}

	// 设置存活时间
	if (LifeTime > 0.0f)
	{
		SetLifeSpan(LifeTime);
	}

	// 初始化掉落动画
	DropStartLocation = GetActorLocation();
	DropTargetLocation = DropStartLocation;
	DropTargetLocation.Z = DropStartLocation.Z - DropBounceHeight; // 先下落
	InitialZ = DropTargetLocation.Z;

	UE_LOG(LogTemp, Log, TEXT("[GoldPickup] Spawned with %d gold"), GoldAmount);
}

void AGoldPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 如果已被拾取，不再执行任何逻辑
	if (bIsPickedUp)
	{
		return;
	}

	// ===== 吸附逻辑（优先级最高）=====
	if (bIsBeingAttracted && AttractTarget)
	{
		// 获取玩家腰间位置作为吸附目标
		FVector TargetLocation = AttractTarget->GetActorLocation();
		TargetLocation.Z += AttractHeightOffset;  // 提高到腰间高度

		FVector CurrentLocation = GetActorLocation();
		float Distance = FVector::Dist(CurrentLocation, TargetLocation);

		// 如果足够近，立即拾取
		if (Distance < AutoPickupRadius)
		{
			PickUp(AttractTarget);
			return;  // 拾取后立即退出
		}

		// 移动向玩家腰间
		FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
		FVector NewLocation = CurrentLocation + Direction * AttractSpeed * DeltaTime;
		SetActorLocation(NewLocation);

		// 吸附时仍然旋转（加速旋转效果）
		FRotator CurrentRotation = GetActorRotation();
		CurrentRotation.Yaw += RotationSpeed * 2.0f * DeltaTime;
		SetActorRotation(CurrentRotation);

		return;  // 关键：吸附状态下不执行后续的浮动效果
	}

	// ===== 掉落弹跳动画 =====
	if (!bHasLanded)
	{
		DropTimer += DeltaTime;
		float Alpha = FMath::Clamp(DropTimer / DropDuration, 0.0f, 1.0f);

		// 使用正弦曲线模拟弹跳
		float BounceAlpha = FMath::Sin(Alpha * PI);
		float CurrentZ = FMath::Lerp(DropStartLocation.Z, DropTargetLocation.Z, Alpha);
		CurrentZ += BounceAlpha * DropBounceHeight * 0.5f;  // 弹跳效果

		FVector NewLocation = GetActorLocation();
		NewLocation.Z = CurrentZ;
		SetActorLocation(NewLocation);

		if (Alpha >= 1.0f)
		{
			bHasLanded = true;
			InitialZ = GetActorLocation().Z;
		}
	}
	else
	{
		// ===== 待机状态：旋转和浮动效果 =====
		// 旋转效果
		FRotator CurrentRotation = GetActorRotation();
		CurrentRotation.Yaw += RotationSpeed * DeltaTime;
		SetActorRotation(CurrentRotation);

		// 浮动效果
		FloatTimer += DeltaTime;
		float FloatOffset = FMath::Sin(FloatTimer * FloatFrequency * 2.0f * PI) * FloatAmplitude;
		FVector NewLocation = GetActorLocation();
		NewLocation.Z = InitialZ + FloatOffset;
		SetActorLocation(NewLocation);
	}
}

void AGoldPickup::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 检查是否是玩家
	AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// 如果已经在吸附或等待状态，不再处理
	if (bIsBeingAttracted || bWaitingForPickup)
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	// 进入吸附范围时，等待玩家按F拾取
	if (Distance <= AttractRadius)
	{
		bWaitingForPickup = true;
		NearbyPlayer = Player;
		// 通知玩家有金币可拾取
		Player->SetNearbyGold(this);
		UE_LOG(LogTemp, Log, TEXT("[GoldPickup] Player entered range, waiting for pickup"));
	}
}

// 玩家离开检测范围
void AGoldPickup::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
	if (!Player || Player != NearbyPlayer)
	{
		return;
	}

	// 如果正在吸附中，不处理离开事件
	if (bIsBeingAttracted)
	{
		return;
	}

	// 清除等待状态
	bWaitingForPickup = false;
	NearbyPlayer = nullptr;
	// 通知玩家离开金币范围
	Player->SetNearbyGold(nullptr);
	UE_LOG(LogTemp, Log, TEXT("[GoldPickup] Player left range"));
}

// 开始吸附（玩家按F键时调用）
void AGoldPickup::StartAttract()
{
	if (bWaitingForPickup && NearbyPlayer && !bIsBeingAttracted)
	{
		bIsBeingAttracted = true;
		AttractTarget = NearbyPlayer;
		bWaitingForPickup = false;
		UE_LOG(LogTemp, Log, TEXT("[GoldPickup] Started attracting to player"));
	}
}

void AGoldPickup::PickUp(AWukongCharacter* Player)
{
	// 防止重复拾取
	if (bIsPickedUp || !Player)
	{
		return;
	}

	// 标记为已拾取
	bIsPickedUp = true;

	// 获取玩家的金币组件
	UWalletComponent* Wallet = Player->GetWalletComponent();
	if (Wallet)
	{
		Wallet->AddGold(GoldAmount);
		UE_LOG(LogTemp, Log, TEXT("[GoldPickup] Player picked up %d gold. Total: %d"),
			GoldAmount, Wallet->GetGold());
	}

	// 播放拾取音效
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	// 播放拾取特效
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			PickupEffect,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// 销毁自身
	Destroy();
}
