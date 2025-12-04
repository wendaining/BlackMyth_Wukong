// 伤害类型定义 - 用于描述伤害信息

#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.generated.h"

/**
 * 伤害类型枚举
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical    UMETA(DisplayName = "物理"),
	Fire        UMETA(DisplayName = "火焰"),
	Ice         UMETA(DisplayName = "冰冻"),
	Lightning   UMETA(DisplayName = "雷电"),
	Poison      UMETA(DisplayName = "毒"),
	Pure        UMETA(DisplayName = "真实伤害")  // 无视护甲
};

/**
 * 攻击类型枚举
 */
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Light       UMETA(DisplayName = "轻攻击"),
	Heavy       UMETA(DisplayName = "重攻击"),
	Air         UMETA(DisplayName = "空中攻击"),
	Ability     UMETA(DisplayName = "技能"),
	Projectile  UMETA(DisplayName = "投射物"),
	Area        UMETA(DisplayName = "范围")
};

/**
 * 伤害信息结构体
 * 包含一次攻击的所有信息
 */
USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	/** 基础伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.0f;

	/** 伤害类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EDamageType DamageType = EDamageType::Physical;

	/** 攻击类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EAttackType AttackType = EAttackType::Light;

	/** 造成伤害的 Actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	AActor* Instigator = nullptr;

	/** 造成伤害的具体物体（武器、投射物等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	AActor* DamageCauser = nullptr;

	/** 是否暴击 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bIsCritical = false;

	/** 暴击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float CriticalMultiplier = 1.5f;

	/** 受击方向（世界空间） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector HitDirection = FVector::ZeroVector;

	/** 受击点（世界空间） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector HitLocation = FVector::ZeroVector;

	/** 连击索引 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 ComboIndex = 0;

	/** 是否可以被格挡 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanBeBlocked = true;

	/** 是否可以被闪避 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanBeDodged = true;

	/** 击退力度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float KnockbackForce = 0.0f;

	/** 硬直时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float HitStunDuration = 0.2f;

	// 默认构造函数
	FDamageInfo() {}

	// 便捷构造函数
	FDamageInfo(float InDamage, AActor* InInstigator, EAttackType InAttackType = EAttackType::Light)
		: BaseDamage(InDamage)
		, AttackType(InAttackType)
		, Instigator(InInstigator)
	{}

	/** 获取最终伤害（包含暴击） */
	float GetFinalDamage() const
	{
		return bIsCritical ? BaseDamage * CriticalMultiplier : BaseDamage;
	}
};
