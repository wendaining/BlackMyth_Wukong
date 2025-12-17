#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EnemyBase.h"
#include "BlackMythSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FEnemySaveData
{
    GENERATED_BODY()

public:
    // 唯一标识敌人
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    FGuid EnemyID = FGuid::NewGuid();

    // 敌人蓝图类
    UPROPERTY()
    TSubclassOf<AEnemyBase> EnemyClass = nullptr;

    // 等级
    UPROPERTY()
    int32 Level = 1;

    // 基础状态
    UPROPERTY()
    bool bIsDead = false;

    UPROPERTY()
    float CurrentHealth = 100.f; // 可按敌人默认血量修改

    UPROPERTY()
    float CurrentPoise = 0.f;

    UPROPERTY()
    EEnemyState EnemyState = EEnemyState::EES_NoState;

    // 位置与旋转
    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    FRotator Rotation = FRotator::ZeroRotator;

    // 眩晕状态
    UPROPERTY()
    bool bIsStunned = false;

    UPROPERTY()
    float StunRemainingTime = 0.f;

    // 定身状态
    UPROPERTY()
    bool bIsFrozen = false;

    UPROPERTY()
    float FrozenAnimPosition = 0.f;

    UPROPERTY()
    EEnemyState StateBeforeFreeze = EEnemyState::EES_NoState;

    UPROPERTY()
    float MovementSpeedBeforeFreeze = 0.f;

    // 武器信息
    UPROPERTY()
    TSubclassOf<AActor> WeaponClass = nullptr;

    UPROPERTY()
    FName WeaponSocketName = FName("weapon_r");
};

UCLASS()
class BLACKMYTH_API UBlackMythSaveGame : public USaveGame {
  GENERATED_BODY()

 public:
  /** 玩家在世界中的位置。 */
  UPROPERTY()
  FVector PlayerLocation;

  /** 玩家的旋转角度。 */
  UPROPERTY()
  FRotator PlayerRotation;

  /** 当前血量值。 */
  UPROPERTY()
  float PlayerHealth = 100.0f;

  /** 最大血量值。 */
  UPROPERTY()
  float PlayerMaxHealth = 100.0f;

  /** 当前体力值。 */
  UPROPERTY()
  float PlayerStamina = 100.0f;

  /** 最大体力值。 */
  UPROPERTY()
  float PlayerMaxStamina = 100.0f;

  /** 存档显示名称。 */
  UPROPERTY()
  FString SaveName;

  /** 存档创建时间。 */
  UPROPERTY()
  FDateTime SaveTime;

  // 敌人数据
  UPROPERTY()

  TArray<FEnemySaveData> Enemies;
};
