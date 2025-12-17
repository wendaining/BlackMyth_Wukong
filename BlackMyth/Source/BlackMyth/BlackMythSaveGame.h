#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BlackMythSaveGame.generated.h"

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
};
