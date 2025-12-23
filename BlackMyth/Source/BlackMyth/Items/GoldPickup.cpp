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

	// 掉落弹跳动画
	if (!bHasLanded)
	{
		DropTimer += DeltaTime;
		float Alpha = FMath::Clamp(DropTimer / DropDuration, 0.0f, 1.0f);

		// 使用正弦曲线模拟弹跳
		float BounceAlpha = FMath::Sin(Alpha * PI);
		float CurrentZ = FMath::Lerp(DropStartLocation.Z, DropTargetLocation.Z, Alpha);
		CurrentZ += BounceAlpha * DropBounceHeight * 0.5f; // 弹跳效果

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

	// 吸附到玩家
	if (bIsBeingAttracted && AttractTarget)
	{
		FVector TargetLocation = AttractTarget->GetActorLocation();
		TargetLocation.Z += AttractHeightOffset;  // 提高到腰间高度

		FVector CurrentLocation = GetActorLocation();
		FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
		float Distance = FVector::Dist(CurrentLocation, TargetLocation);

		// 移动向玩家
		FVector NewLocation = CurrentLocation + Direction * AttractSpeed * DeltaTime;
		SetActorLocation(NewLocation);

		// 如果足够近，自动拾取
		if (Distance < AutoPickupRadius)
		{
			PickUp(AttractTarget);
			return;
		}
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

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	// 如果在自动拾取范围内，直接拾取
	if (bAutoPickup && Distance <= AutoPickupRadius)
	{
		PickUp(Player);
	}
	// 如果在吸附范围内，开始吸附
	else if (Distance <= AttractRadius)
	{
		bIsBeingAttracted = true;
		AttractTarget = Player;
	}
}

void AGoldPickup::PickUp(AWukongCharacter* Player)
{
	if (!Player)
	{
		return;
	}

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
