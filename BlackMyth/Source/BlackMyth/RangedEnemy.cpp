#include "RangedEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ARangedEnemy::ARangedEnemy()
{
	// 远程敌人通常移动速度稍慢，或者血量稍低
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	
	// 设置默认的射击距离
	RangedAttackDistance = 1000.0f;
}
