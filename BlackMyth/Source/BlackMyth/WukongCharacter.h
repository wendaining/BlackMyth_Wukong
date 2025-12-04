// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackMythCharacter.h"
#include "WukongCharacter.generated.h"

class UInputAction;
struct FInputActionValue;

// Character state enum
UENUM(BlueprintType)
enum class EWukongState : uint8
{
	Idle,
	Moving,
	Attacking,
	Dodging,
	UsingAbility,  // 使用战技中
	HitStun,
	Dead
};

// Health changed delegate - declared before class
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

UCLASS()
class BLACKMYTH_API AWukongCharacter : public ABlackMythCharacter
{
	GENERATED_BODY()

public:
	AWukongCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Public Interface
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveDamage(float Damage, AActor* DamageInstigator);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvincible(bool bInInvincible);

	UFUNCTION(BlueprintPure, Category = "State")
	EWukongState GetCurrentState() const { return CurrentState; }

	// Animation accessors
	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetComboIndex() const { return CurrentComboIndex; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetMovementSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetMovementDirection() const;

	// Health delegate
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHealthChanged OnHealthChanged;

protected:
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AbilityAction;  // Q键战技

	// Combat Component - TODO: Uncomment when UCombatComponent is implemented by Member C
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	// TObjectPtr<UCombatComponent> CombatComponent;

	// Character Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.0f;

	// Dodge Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeInvincibilityDuration = 0.3f;

	// Attack Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxComboCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ComboResetTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float InputBufferTime = 0.3f;

	/** 攻击冷却时间 - 防止点击过快导致攻击过于频繁 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 0.4f;

	// Hit Stun Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HitStunDuration = 0.5f;

	// Animation Montages - Attack
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage1;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage2;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage3;

	// Animation Montages - Movement
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Animation Sequences - For reference or creating dynamic montages
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkForwardAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintForwardAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpStartAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpApexAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpLandAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> DodgeAnimation;

	// 受击动画
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactFrontAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactBackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactRightAnimation;

	// 死亡动画
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> DeathAnimation;

	// 击退/击飞动画
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> KnockbackAnimation;

	// 眩晕动画
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> StunStartAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> StunLoopAnimation;

	// 表情动画
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Emote")
	TObjectPtr<UAnimSequence> EmoteTauntAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Emote")
	TObjectPtr<UAnimSequence> EmoteStaffSpinAnimation;

	// 技能动画（Q技能 - 翻滚冲击）
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimSequence> AbilityFlipForwardAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimSequence> AbilitySlamAnimation;

	// 空中攻击
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> AirAttackAnimation;

	// 战技蒙太奇（Q技能 - 筋斗云突击）
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimMontage> AbilityMontage;

	// 战技属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityCooldown = 5.0f;  // 战技冷却时间

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityDamage = 50.0f;  // 战技伤害

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityRadius = 300.0f;  // AOE范围

	/**
	 * 攻击蒙太奇容器，使用 TObjectPtr 遵循 UE5 推荐的智能指针写法，
	 * 可让 GC/反射系统追踪引用，避免蓝图或热重载导致的悬挂指针。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/**
	 * 触发一次攻击，后续会随机挑选 AttackMontages 中的动画蒙太奇进行播放。
	 */
	void Attack();

private:
	// State Management
	EWukongState CurrentState = EWukongState::Idle;
	EWukongState PreviousState = EWukongState::Idle;

	// Invincibility
	bool bIsInvincible = false;
	float InvincibilityTimer = 0.0f;

	// Dodge State
	bool bIsDodging = false;
	float DodgeTimer = 0.0f;
	FVector DodgeDirection;
	TMap<FString, float> CooldownMap;

	// Attack State
	int32 CurrentComboIndex = 0;
	float LastAttackTime = 0.0f;
	float AttackTimer = 0.0f;
	float AttackCooldownTimer = 0.0f;  // 攻击冷却计时器
	float CachedMaxWalkSpeed = 0.0f;   // 攻击时缓存的最大速度
	TArray<FString> InputBuffer;

	// Hit Stun State
	float HitStunTimer = 0.0f;

	// Sprint State
	bool bIsSprinting = false;

	// Input Handlers
	void OnDodgePressed();
	void OnAttackPressed();
	void OnSprintStarted();
	void OnSprintStopped();
	void OnAbilityPressed();  // Q键战技

	// State Methods
	void ChangeState(EWukongState NewState);
	void UpdateState(float DeltaTime);
	void UpdateIdleState(float DeltaTime);
	void UpdateMovingState(float DeltaTime);
	void UpdateAttackingState(float DeltaTime);
	void UpdateDodgingState(float DeltaTime);
	void UpdateAbilityState(float DeltaTime);  // 战技状态更新
	void UpdateHitStunState(float DeltaTime);
	void UpdateDeadState(float DeltaTime);

	// Combat Methods
	void PerformAttack();
	void ResetCombo();
	void ProcessInputBuffer();

	// Dodge Methods
	void PerformDodge();
	void UpdateDodgeMovement(float DeltaTime);

	// Ability Methods
	void PerformAbility();  // 执行战技

	// 战技状态
	bool bIsUsingAbility = false;
	float AbilityTimer = 0.0f;

	// Cooldown Management
	bool IsCooldownActive(const FString& CooldownName) const;
	void StartCooldown(const FString& CooldownName, float Duration);
	void UpdateCooldowns(float DeltaTime);

	// Helper Methods
	void Die();
	void UpdateMovementSpeed();
	FVector GetMovementInputDirection() const;

	/**
	 * 从 Animation Sequence 动态创建并播放 Montage
	 * 这样可以避免修改原始 Montage 资源，同时使用自定义的 Slot
	 * 
	 * @param AnimSequence 要播放的动画序列
	 * @param SlotName 要使用的 Slot 名称（默认为 DefaultSlot）
	 * @param PlayRate 播放速率
	 * @return 播放的 Montage 时长，失败返回 0
	 */
	float PlayAnimationAsMontageDynamic(UAnimSequence* AnimSequence, FName SlotName = FName("DefaultSlot"), float PlayRate = 1.0f);
};
