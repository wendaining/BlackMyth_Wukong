// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

class UDialogueComponent;

/**
 * NPC基类
 * - 继承自ACharacter
 * - 属于中立阵营，无法被攻击
 * - 可以携带对话数据
 */
UCLASS()
class BLACKMYTH_API ANPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCCharacter();

protected:
	virtual void BeginPlay() override;

public:
	// 对话组件（管理该NPC的所有对话）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueComponent* DialogueComponent;

	// 是否可以交互（可以在蓝图中动态控制）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract;

	// 交互距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance;

	// 检查是否可以与该NPC交互
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanBeInteractedWith() const;

	// 开始对话
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue();

protected:
	// 设置中立阵营
	virtual void PostInitializeComponents() override;
};
