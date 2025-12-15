// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCCharacter.h"
#include "Dialogue/DialogueComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建对话组件
	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

	// 默认可以交互
	bCanInteract = true;
	InteractionDistance = 300.0f;

	// 设置碰撞：保持WorldDynamic通道响应，用于检测
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldDynamic);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 允许玩家重叠检测
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	// 禁用移动
	GetCharacterMovement()->DisableMovement();
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANPCCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 设置为中立阵营（Team ID = 255表示中立）
	// 注：如果项目中使用了GenericTeamId系统，可以在这里设置
	// 目前先通过碰撞设置来实现"无法被攻击"
}

bool ANPCCharacter::CanBeInteractedWith() const
{
	return bCanInteract && DialogueComponent != nullptr;
}

void ANPCCharacter::StartDialogue()
{
	if (DialogueComponent && CanBeInteractedWith())
	{
		DialogueComponent->StartDialogue();
	}
}
