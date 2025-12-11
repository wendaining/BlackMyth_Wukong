#include "RangedEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ARangedEnemy::ARangedEnemy()
{
	// 远程敌人通常移动速度稍慢，或者血量稍低
	// 注意：EnemyBase 会在状态切换时重置 MaxWalkSpeed，所以我们需要设置 PatrollingSpeed 和 ChasingSpeed
	PatrollingSpeed = 200.0f;
	ChasingSpeed = 400.0f;
	
	// 设置默认的射击距离
	RangedAttackDistance = 1000.0f;

	// 关键修正：将 AttackRadius 设置为射程
	// 这样 EnemyBase 的 Tick 逻辑会在距离 <= AttackRadius (1000) 时停止移动并开始攻击
	AttackRadius = RangedAttackDistance;
}
