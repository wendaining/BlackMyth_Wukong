#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "RegularEnemy.generated.h"

/**
 * 普通敌人
 */
UCLASS()
class BLACKMYTH_API ARegularEnemy : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	ARegularEnemy();

protected:
	virtual void BeginPlay() override;
};
