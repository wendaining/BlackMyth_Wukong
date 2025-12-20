// 药瓶Actor父类实现

#include "PotionActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

APotionActor::APotionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建根组件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 创建药瓶网格体组件
	PotionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
	PotionMesh->SetupAttachment(RootComponent);
	PotionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 不需要碰撞
	PotionMesh->SetVisibility(false);  // 默认隐藏
}

void APotionActor::BeginPlay()
{
	Super::BeginPlay();
}

void APotionActor::ShowPotion()
{
	if (PotionMesh)
	{
		PotionMesh->SetVisibility(true);
		UE_LOG(LogTemp, Log, TEXT("PotionActor: %s shown"), *PotionTypeName);
	}
}

void APotionActor::HidePotion()
{
	if (PotionMesh)
	{
		PotionMesh->SetVisibility(false);
		UE_LOG(LogTemp, Log, TEXT("PotionActor: %s hidden"), *PotionTypeName);
	}
}

void APotionActor::AttachToCharacter(ACharacter* Character)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("PotionActor::AttachToCharacter - Character is null!"));
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("PotionActor::AttachToCharacter - Character mesh is null!"));
		return;
	}

	// 附加到角色的指定骨骼
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(CharacterMesh, AttachRules, AttachSocketName);

	UE_LOG(LogTemp, Log, TEXT("PotionActor: %s attached to %s at socket %s"),
		*PotionTypeName, *Character->GetName(), *AttachSocketName.ToString());
}

void APotionActor::DetachAndDestroy()
{
	// 分离
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 延迟销毁（0.1秒后）
	SetLifeSpan(0.1f);

	UE_LOG(LogTemp, Log, TEXT("PotionActor: %s detached and scheduled for destruction"), *PotionTypeName);
}
