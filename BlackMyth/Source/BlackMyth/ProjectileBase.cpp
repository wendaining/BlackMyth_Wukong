#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 创建碰撞球体
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(20.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile")); // 确保项目中有 Projectile 碰撞预设，或者使用 BlockAllDynamic
	// 如果没有 Projectile 预设，手动设置碰撞响应
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 对 Pawn 使用 Overlap 以便触发 OnHit 但不推挤
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 2. 创建网格体
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 网格体不参与碰撞，只负责显示

	// 3. 创建运动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.0f;
	ProjectileMovement->MaxSpeed = 1500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 让投掷物朝向飞行方向
	ProjectileMovement->bShouldBounce = false; // 默认不反弹
	ProjectileMovement->ProjectileGravityScale = 1.0f; // 正常重力

	// 默认存活时间 (防止无限飞行)
	InitialLifeSpan = 5.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 绑定碰撞事件
	CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 忽略自己和拥有者
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// 造成伤害
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

		// 播放特效
		if (HitParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, GetActorLocation(), GetActorRotation());
		}

		// 播放音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}

		// 销毁投掷物
		Destroy();
	}
}
