// 测试靶子实现

#include "TargetDummy.h"
#include "../Components/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

ATargetDummy::ATargetDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 根组件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// 碰撞体
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
	CollisionComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	// 网格体（使用引擎内置立方体）
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	MeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 2.0f));

	// 设置默认网格（立方体）
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	// 生命组件
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ATargetDummy::BeginPlay()
{
	Super::BeginPlay();

	// 创建动态材质
	if (MeshComponent && MeshComponent->GetMaterial(0))
	{
		DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
		UpdateMaterialColor();
	}

	// 绑定生命组件事件
	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddDynamic(this, &ATargetDummy::OnDamageTaken);
		HealthComponent->OnDeath.AddDynamic(this, &ATargetDummy::OnDeath);
	}

	UE_LOG(LogTemp, Log, TEXT("[TargetDummy] %s spawned with %.0f HP"), *GetName(), HealthComponent->GetMaxHealth());
}

void ATargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 受击闪烁效果
	if (HitFlashTimer > 0.0f)
	{
		HitFlashTimer -= DeltaTime;
		if (HitFlashTimer <= 0.0f)
		{
			UpdateMaterialColor();
		}
	}
}

void ATargetDummy::ResetDummy()
{
	bIsDead = false;
	
	if (HealthComponent)
	{
		HealthComponent->FullHeal();
	}

	// 恢复碰撞
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	UpdateMaterialColor();

	UE_LOG(LogTemp, Log, TEXT("[TargetDummy] %s Reset!"), *GetName());
}

void ATargetDummy::OnDamageTaken(float Damage, AActor* DamageInstigator, float RemainingHealth)
{
	// 闪烁效果
	HitFlashTimer = 0.15f;
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), HitColor);
	}

	// 屏幕日志
	if (bShowDamageNumbers)
	{
		FString InstigatorName = DamageInstigator ? DamageInstigator->GetName() : TEXT("Unknown");
		GEngine->AddOnScreenDebugMessage(
			-1, 
			2.0f, 
			FColor::Orange,
			FString::Printf(TEXT("[%s] Hit by %s! Damage: %.1f | HP: %.1f / %.1f"), 
				*GetName(), 
				*InstigatorName, 
				Damage, 
				RemainingHealth,
				HealthComponent->GetMaxHealth())
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("[TargetDummy] %s took %.1f damage from %s. HP: %.1f"), 
		*GetName(), Damage, DamageInstigator ? *DamageInstigator->GetName() : TEXT("Unknown"), RemainingHealth);
}

void ATargetDummy::OnDeath(AActor* Killer)
{
	bIsDead = true;

	// 更新颜色
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DeadColor);
	}

	// 禁用碰撞
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 屏幕日志
	FString KillerName = Killer ? Killer->GetName() : TEXT("Unknown");
	GEngine->AddOnScreenDebugMessage(
		-1, 
		3.0f, 
		FColor::Red,
		FString::Printf(TEXT("[%s] DESTROYED by %s!"), *GetName(), *KillerName)
	);

	UE_LOG(LogTemp, Error, TEXT("[TargetDummy] %s DESTROYED by %s!"), *GetName(), *KillerName);

	// 自动重生
	if (RespawnTime > 0.0f)
	{
		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(
			RespawnTimer,
			this,
			&ATargetDummy::ResetDummy,
			RespawnTime,
			false
		);

		GEngine->AddOnScreenDebugMessage(
			-1, 
			RespawnTime, 
			FColor::Yellow,
			FString::Printf(TEXT("[%s] Respawning in %.1f seconds..."), *GetName(), RespawnTime)
		);
	}
}

void ATargetDummy::UpdateMaterialColor()
{
	if (!DynamicMaterial)
	{
		return;
	}

	if (bIsDead)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DeadColor);
	}
	else
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NormalColor);
	}
}
