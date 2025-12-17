// 蝴蝶Pawn - 变身术使用

#include "ButterflyPawn.h"
#include "WukongCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AButterflyPawn::AButterflyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建碰撞球体作为根组件（这样才能有碰撞）
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(30.0f); // 蝴蝶大小的碰撞球
	CollisionSphere->SetCollisionProfileName(TEXT("Pawn"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 忽略摄像机
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 忽略可见性检测
	RootComponent = CollisionSphere;

	// 创建蝴蝶Mesh（作为子组件，可以调整旋转）
	ButterflyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ButterflyMesh"));
	ButterflyMesh->SetupAttachment(RootComponent);
	ButterflyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Mesh不需要碰撞，球体处理

	// 创建飞行移动组件
	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	FloatingMovement->MaxSpeed = FlySpeed;
	FloatingMovement->Acceleration = 2000.0f;
	FloatingMovement->Deceleration = 4000.0f;

	// 创建摄像机臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true; // 跟随控制器旋转
	CameraBoom->bDoCollisionTest = false; // 蝴蝶很小，不需要碰撞检测
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = false;

	// 创建跟随摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	// Pawn设置
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; // 蝴蝶跟随视角Yaw旋转
	bUseControllerRotationRoll = false;

	// 初始化输入
	CurrentMoveInput = FVector2D::ZeroVector;
	CurrentLookInput = FVector2D::ZeroVector;
	TransformTimeRemaining = 0.0f;
}

void AButterflyPawn::BeginPlay()
{
	Super::BeginPlay();

	// 应用Mesh旋转偏移（修正模型朝向）
	if (ButterflyMesh)
	{
		ButterflyMesh->SetRelativeRotation(MeshRotationOffset);
		UE_LOG(LogTemp, Log, TEXT("[Butterfly] Applied MeshRotationOffset: Pitch=%.1f, Yaw=%.1f, Roll=%.1f"), 
			MeshRotationOffset.Pitch, MeshRotationOffset.Yaw, MeshRotationOffset.Roll);
	}

	// 添加输入映射上下文
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (ButterflyMappingContext)
			{
				// 添加蝴蝶专用的输入映射（高优先级）
				Subsystem->AddMappingContext(ButterflyMappingContext, 1);
				UE_LOG(LogTemp, Log, TEXT("[Butterfly] Added ButterflyMappingContext"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Butterfly] ButterflyMappingContext is NULL! Set it in BP_Butterfly_Hyoumon."));
			}
		}
	}

	// 更新飞行速度
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = FlySpeed;
	}

	// 更新摄像机距离
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = CameraDistance;
		UE_LOG(LogTemp, Log, TEXT("[Butterfly] Set CameraBoom->TargetArmLength = %.1f"), CameraDistance);
	}

	UE_LOG(LogTemp, Log, TEXT("[Butterfly] BeginPlay - FlySpeed=%.1f, CameraDistance=%.1f, TransformDuration=%.1f"), 
		FlySpeed, CameraDistance, TransformDuration);
}

void AButterflyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 处理变身计时器
	if (TransformTimeRemaining > 0.0f)
	{
		TransformTimeRemaining -= DeltaTime;
		
		// 每5秒输出一次剩余时间（调试用）
		static float LastLogTime = 0.0f;
		if (FMath::FloorToInt(TransformTimeRemaining) % 5 == 0 && 
			FMath::Abs(TransformTimeRemaining - FMath::FloorToInt(TransformTimeRemaining)) < DeltaTime)
		{
			UE_LOG(LogTemp, Log, TEXT("[Butterfly] Transform time remaining: %.1f seconds"), TransformTimeRemaining);
		}
		
		if (TransformTimeRemaining <= 0.0f)
		{
			// 时间到，变回悟空
			TransformTimeRemaining = 0.0f;
			UE_LOG(LogTemp, Log, TEXT("[Butterfly] Transform duration ended! OwnerWukong=%p"), OwnerWukong.Get());
			
			if (OwnerWukong)
			{
				UE_LOG(LogTemp, Log, TEXT("[Butterfly] Calling TransformBackToWukong on %s..."), *OwnerWukong->GetName());
				OwnerWukong->TransformBackToWukong();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[Butterfly] OwnerWukong is NULL! Cannot transform back!"));
			}
			return; // 变回后不再处理输入
		}
	}

	// 处理视角旋转
	if (!CurrentLookInput.IsNearlyZero())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 水平旋转（Yaw）
			PC->AddYawInput(CurrentLookInput.X * LookSensitivity);
			// 垂直旋转（Pitch）
			PC->AddPitchInput(-CurrentLookInput.Y * LookSensitivity);
		}
	}

	// 处理飞行移动
	if (!CurrentMoveInput.IsNearlyZero() && FloatingMovement)
	{
		// 获取控制器的旋转（视角方向）
		FRotator ControlRotation = GetControlRotation();
		
		// 计算前进方向（考虑Pitch，这样看向上时按W会往上飞）
		FVector ForwardDir = ControlRotation.Vector();
		
		// 计算右方向（仅用于左右移动，不考虑Pitch）
		FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
		FVector RightDir = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::Y);

		// 计算移动方向
		FVector MoveDirection = ForwardDir * CurrentMoveInput.Y + RightDir * CurrentMoveInput.X;
		MoveDirection.Normalize();

		// 添加移动输入
		FloatingMovement->AddInputVector(MoveDirection);
	}
}

void AButterflyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemp, Log, TEXT("[Butterfly] SetupPlayerInputComponent called"));

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定移动
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AButterflyPawn::OnMove);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AButterflyPawn::OnMove);
			UE_LOG(LogTemp, Log, TEXT("[Butterfly] Bound MoveAction"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Butterfly] MoveAction is NULL!"));
		}

		// 绑定视角
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AButterflyPawn::OnLook);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AButterflyPawn::OnLook);
			UE_LOG(LogTemp, Log, TEXT("[Butterfly] Bound LookAction"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Butterfly] LookAction is NULL!"));
		}
	}
}

void AButterflyPawn::OnMove(const FInputActionValue& Value)
{
	CurrentMoveInput = Value.Get<FVector2D>();
}

void AButterflyPawn::OnLook(const FInputActionValue& Value)
{
	CurrentLookInput = Value.Get<FVector2D>();
}

void AButterflyPawn::InitializeTransform(AWukongCharacter* InOwner, float InDuration)
{
	OwnerWukong = InOwner;
	TransformDuration = InDuration;
	TransformTimeRemaining = InDuration;
	
	UE_LOG(LogTemp, Log, TEXT("[Butterfly] InitializeTransform - Owner=%s (Ptr=%p), Duration=%.1f seconds"), 
		InOwner ? *InOwner->GetName() : TEXT("NULL"), InOwner, InDuration);
	UE_LOG(LogTemp, Log, TEXT("[Butterfly] InitializeTransform - OwnerWukong after assignment: %p"), OwnerWukong.Get());
}
