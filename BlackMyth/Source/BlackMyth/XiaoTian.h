#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XiaoTian.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class EXiaoTianState : uint8
{
	Spawning,
	Flying,
	Biting,
	Finished
};

UCLASS()
class BLACKMYTH_API AXiaoTian : public AActor
{
	GENERATED_BODY()
	
public:	
	AXiaoTian();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = "XiaoTian")
	void PlayEndAndVanish();

protected:
	/** 彻底销毁的回调 (用于相机混合后的延迟销毁) */
	void DestroyActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	// ========== 状态配置 ==========
	
	EXiaoTianState CurrentState = EXiaoTianState::Spawning;

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Movement")
	float FlightSpeed = 2200.0f;

	/** 起跳多久后开始飞 (用于对齐动画最帅的那一帧) */
	UPROPERTY(EditAnywhere, Category = "XiaoTian|Movement")
	float LaunchDelay = 0.35f;

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Movement")
	float MaxFlightDistance = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Combat")
	float Damage = 20.0f;

	// ========== 动画蒙太奇 ==========

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Animation")
	TObjectPtr<UAnimMontage> PounceStartMontage;

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Animation")
	TObjectPtr<UAnimMontage> PounceLoopMontage;

	UPROPERTY(EditAnywhere, Category = "XiaoTian|Animation")
	TObjectPtr<UAnimMontage> BiteEndMontage;

	/** 消失时的特效 (VFX) */
	UPROPERTY(EditAnywhere, Category = "XiaoTian|Visuals")
	TObjectPtr<class UNiagaraSystem> VanishFX;

	// ========== 内部逻辑 ==========

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void StartFlying();
	void TriggerBite(AActor* Target);
	void FinishAndDestroy();

	FVector TargetDirection;
	FVector SpawnLocation;
	FTimerHandle LifeTimer;
};
