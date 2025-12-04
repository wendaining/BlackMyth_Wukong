// 悟空角色类 - 黑神话悟空项目

#pragma once

#include "CoreMinimal.h"
#include "BlackMythCharacter.h"
#include "WukongCharacter.generated.h"

class UInputAction;
class UStaminaComponent;
struct FInputActionValue;

// 角色状态枚举
UENUM(BlueprintType)
enum class EWukongState : uint8
{
	Idle,         // 待机
	Moving,       // 移动中
	Attacking,    // 攻击中
	Dodging,      // 翻滚中
	UsingAbility, // 使用战技中
	HitStun,      // 受击硬直
	Dead          // 死亡
};

// 生命值变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

// 体力值变化委托已迁移到 UStaminaComponent

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

	// 重写跳跃函数以添加体力检查
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	// ========== 公共接口 ==========
	
	/** 受到伤害 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveDamage(float Damage, AActor* DamageInstigator);

	/** 设置无敌状态 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvincible(bool bInInvincible);

	/** 获取当前状态 */
	UFUNCTION(BlueprintPure, Category = "State")
	EWukongState GetCurrentState() const { return CurrentState; }

	// ========== 动画访问器 ==========
	
	/** 获取当前连击索引 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetComboIndex() const { return CurrentComboIndex; }

	/** 计算最终伤害值 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float CalculateDamage(bool bIsHeavyAttack = false, bool bIsAirAttack = false, int32 ComboIndex = 0) const;

	/** 获取基础攻击力 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetBaseAttackPower() const { return BaseAttackPower; }

	/** 是否正在冲刺 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	/** 获取移动速度 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetMovementSpeed() const;

	/** 获取移动方向 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetMovementDirection() const;

	// 生命值变化委托
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHealthChanged OnHealthChanged;

	/** 获取体力组件 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

protected:
	// ========== 输入动作 ==========
	
	/** 翻滚输入动作 (C键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;

	/** 攻击输入动作 (鼠标左键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	/** 冲刺输入动作 (Shift键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	/** 战技输入动作 (Q键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AbilityAction;

	// 战斗组件 - TODO: 等 Member C 实现 UCombatComponent 后取消注释
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	// TObjectPtr<UCombatComponent> CombatComponent;

	// ========== 角色属性 ==========
	
	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	/** 当前生命值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttackPower = 10.0f;

	// ========== 体力组件 ==========

	/** 体力值组件（管理体力消耗与恢复） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	// ========== 移动属性 ==========
	
	/** 行走速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	/** 冲刺速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.0f;

	/** 攻击时移动速度倍率（0.5 = 50%速度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AttackMoveSpeedMultiplier = 0.5f;

	// ========== 跳跃属性 ==========
	
	/** 跳跃初速度（越大跳得越高） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float JumpVelocity = 450.0f;

	/** 重力缩放（越大下落越快，默认1.0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float GravityScale = 1.0f;

	/** 空中控制（0=无法改变方向，1=完全控制） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float AirControl = 0.0f;

	/** 空中减速（0=保持惯性不减速，产生抛物线；默认值会让速度快速衰减） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Jump")
	float BrakingDecelerationFalling = 0.0f;

	// ========== 翻滚属性 ==========
	
	/** 翻滚距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDistance = 500.0f;

	/** 翻滚持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDuration = 0.5f;

	/** 翻滚冷却时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeCooldown = 1.0f;

	/** 翻滚无敌时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeInvincibilityDuration = 0.3f;

	// ========== 攻击属性 ==========
	
	/** 最大连击数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxComboCount = 3;

	/** 连击重置时间（超过此时间连击归零） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ComboResetTime = 1.0f;

	// ========== 伤害倍率 ==========

	/** 轻击伤害倍率（基于基础攻击力） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float LightAttackMultiplier = 1.0f;

	/** 重击伤害倍率（基于基础攻击力） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float HeavyAttackMultiplier = 2.0f;

	/** 空中攻击伤害倍率（基于基础攻击力） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float AirAttackMultiplier = 1.8f;

	/** 连击各段伤害倍率 [第1段=1.0, 第2段=1.2, 第3段=1.5] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TArray<float> ComboMultipliers = {1.0f, 1.2f, 1.5f};

	/** 攻击动作持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDuration = 0.6f;

	/** 输入缓冲时间（提前按键的容错时间） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float InputBufferTime = 0.3f;

	/** 攻击冷却时间（防止点击过快） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 0.4f;

	/** 受击硬直时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HitStunDuration = 0.5f;

	// ========== 攻击蒙太奇 ==========
	
	/** 攻击蒙太奇1（连击第1段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage1;

	/** 攻击蒙太奇2（连击第2段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage2;

	/** 攻击蒙太奇3（连击第3段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage3;

	/** 翻滚蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// ========== 移动动画序列 ==========
	
	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> IdleAnimation;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkForwardAnimation;

	/** 冲刺动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintForwardAnimation;

	/** 起跳动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpStartAnimation;

	/** 跳跃最高点动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpApexAnimation;

	/** 落地动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> JumpLandAnimation;

	/** 翻滚动画序列 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> DodgeAnimation;

	// ========== 受击动画 ==========
	
	/** 正面受击 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactFrontAnimation;

	/** 背面受击 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactBackAnimation;

	/** 左侧受击 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactLeftAnimation;

	/** 右侧受击 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> HitReactRightAnimation;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> DeathAnimation;

	/** 击退/击飞动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> KnockbackAnimation;

	// ========== 眩晕动画 ==========
	
	/** 眩晕开始 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> StunStartAnimation;

	/** 眩晕循环 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> StunLoopAnimation;

	// ========== 表情动画 ==========
	
	/** 嘲讽动作 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Emote")
	TObjectPtr<UAnimSequence> EmoteTauntAnimation;

	/** 耍棍动作 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Emote")
	TObjectPtr<UAnimSequence> EmoteStaffSpinAnimation;

	// ========== 战技动画 ==========
	
	/** Q技能动画序列（前空翻） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimSequence> AbilityFlipForwardAnimation;

	/** Q技能动画序列（砸地） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimSequence> AbilitySlamAnimation;

	/** 空中攻击动画序列 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimSequence> AirAttackAnimation;

	/** 地面战技蒙太奇（后空翻） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimMontage> AbilityMontage;

	/** 空中战技蒙太奇（下坠攻击） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimMontage> AirAbilityMontage;

	/** 空中普攻蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Ability")
	TObjectPtr<UAnimMontage> AirAttackMontage;

	// ========== 战技属性 ==========
	
	/** 战技冷却时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityCooldown = 5.0f;

	/** 战技伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityDamage = 50.0f;

	/** 战技AOE范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float AbilityRadius = 300.0f;

	/** 
	 * 攻击蒙太奇容器
	 * 使用 TObjectPtr 遵循 UE5 推荐的智能指针写法
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/** 执行攻击 */
	void Attack();

private:
	// ========== 状态管理 ==========
	EWukongState CurrentState = EWukongState::Idle;   // 当前状态
	EWukongState PreviousState = EWukongState::Idle;  // 上一个状态

	// ========== 无敌状态 ==========
	bool bIsInvincible = false;      // 是否无敌
	float InvincibilityTimer = 0.0f; // 无敌计时器

	// ========== 翻滚状态 ==========
	bool bIsDodging = false;             // 是否正在翻滚
	float DodgeTimer = 0.0f;             // 翻滚计时器
	FVector DodgeDirection;              // 翻滚方向
	TMap<FString, float> CooldownMap;    // 冷却时间表

	// ========== 攻击状态 ==========
	int32 CurrentComboIndex = 0;       // 当前连击索引
	float LastAttackTime = 0.0f;       // 上次攻击时间
	float AttackTimer = 0.0f;          // 攻击计时器
	float AttackCooldownTimer = 0.0f;  // 攻击冷却计时器
	float CachedMaxWalkSpeed = 0.0f;   // 攻击时缓存的最大速度
	TArray<FString> InputBuffer;       // 输入缓冲区

	// ========== 硬直状态 ==========
	float HitStunTimer = 0.0f;  // 硬直计时器

	// ========== 冲刺状态 ==========
	bool bIsSprinting = false;  // 是否正在冲刺

	/** 体力耗尽时的回调 */
	UFUNCTION()
	void OnStaminaDepleted();

	// ========== 战技状态 ==========
	bool bIsUsingAbility = false;  // 是否正在使用战技
	float AbilityTimer = 0.0f;     // 战技计时器

	// ========== 输入处理函数 ==========
	void OnDodgePressed();    // 翻滚按下
	void OnAttackPressed();   // 攻击按下
	void OnSprintStarted();   // 冲刺开始
	void OnSprintStopped();   // 冲刺结束
	void OnAbilityPressed();  // 战技按下

	// ========== 状态更新函数 ==========
	void ChangeState(EWukongState NewState);     // 切换状态
	void UpdateState(float DeltaTime);           // 更新状态
	void UpdateIdleState(float DeltaTime);       // 更新待机状态
	void UpdateMovingState(float DeltaTime);     // 更新移动状态
	void UpdateAttackingState(float DeltaTime);  // 更新攻击状态
	void UpdateDodgingState(float DeltaTime);    // 更新翻滚状态
	void UpdateAbilityState(float DeltaTime);    // 更新战技状态
	void UpdateHitStunState(float DeltaTime);    // 更新硬直状态
	void UpdateDeadState(float DeltaTime);       // 更新死亡状态

	// ========== 战斗函数 ==========
	void PerformAttack();       // 执行攻击
	void ResetCombo();          // 重置连击
	void ProcessInputBuffer();  // 处理输入缓冲

	// ========== 翻滚函数 ==========
	void PerformDodge();                       // 执行翻滚
	void UpdateDodgeMovement(float DeltaTime); // 更新翻滚移动

	// ========== 战技函数 ==========
	void PerformAbility();  // 执行战技

	// ========== 冷却管理 ==========
	bool IsCooldownActive(const FString& CooldownName) const;        // 检查冷却是否激活
	void StartCooldown(const FString& CooldownName, float Duration); // 开始冷却
	void UpdateCooldowns(float DeltaTime);                           // 更新冷却

	// ========== 辅助函数 ==========
	void Die();                                // 死亡处理
	void UpdateMovementSpeed();                // 更新移动速度
	FVector GetMovementInputDirection() const; // 获取移动输入方向

	/**
	 * 从动画序列动态创建并播放蒙太奇
	 * 
	 * @param AnimSequence 要播放的动画序列
	 * @param SlotName 使用的 Slot 名称（默认 DefaultSlot）
	 * @param PlayRate 播放速率
	 * @return 蒙太奇时长，失败返回0
	 */
	float PlayAnimationAsMontageDynamic(UAnimSequence* AnimSequence, FName SlotName = FName("DefaultSlot"), float PlayRate = 1.0f);
};
