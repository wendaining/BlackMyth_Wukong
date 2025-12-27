// 蝴蝶Pawn - 变身术使用

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ButterflyPawn.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UFloatingPawnMovement;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * 蝴蝶Pawn
 * - 用于悟空变身术
 * - 可自由飞行（WASD + 鼠标控制方向）
 * - 敌人无视（Phase 5实现）
 */
UCLASS()
class BLACKMYTH_API AButterflyPawn : public APawn
{
	GENERATED_BODY()

public:
	AButterflyPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========== 组件 ==========
protected:
	/** 碰撞球体（根组件） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** 蝴蝶骨骼网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ButterflyMesh;

	/** 飞行移动组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFloatingPawnMovement> FloatingMovement;

	/** 摄像机臂 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 跟随摄像机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ========== 输入动作 ==========
protected:
	/** 移动输入动作 (WASD) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 视角输入动作 (鼠标) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** 输入映射上下文 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ButterflyMappingContext;

	// ========== 飞行参数 ==========
protected:
	/** 飞行速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FlySpeed = 400.0f;

	/** 视角旋转速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LookSensitivity = 1.0f;

	/** 摄像机距离（拉近让蝴蝶显得小） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance = 150.0f;

	/** Mesh旋转补偿（用于修正蝴蝶模型方向） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FRotator MeshRotationOffset = FRotator(0.0f, -90.0f, 0.0f);

	// ========== 变身计时 ==========
protected:
	/** 变身剩余时间 */
	float TransformTimeRemaining;

	/** 拥有者悟空引用（使用普通指针，因为悟空不会在变身期间被销毁） */
	UPROPERTY()
	TObjectPtr<class AWukongCharacter> OwnerWukong;

public:
	/** 设置拥有者和持续时间 */
	void InitializeTransform(class AWukongCharacter* InOwner, float Duration);

	// ========== 输入处理 ==========
protected:
	/** 处理移动输入 */
	void OnMove(const FInputActionValue& Value);

	/** 处理视角输入 */
	void OnLook(const FInputActionValue& Value);

	/** 当前移动输入 */
	FVector2D CurrentMoveInput;

	/** 当前视角输入 */
	FVector2D CurrentLookInput;

public:
	/** 获取蝴蝶Mesh组件 */
	UFUNCTION(BlueprintPure, Category = "Butterfly")
	USkeletalMeshComponent* GetButterflyMesh() const { return ButterflyMesh; }
};
