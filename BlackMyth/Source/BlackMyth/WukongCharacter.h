// 悟空角色类 - 黑神话悟空项目

#pragma once

#include "CoreMinimal.h"
#include "BlackMythCharacter.h"
#include "WukongCharacter.generated.h"

class UInputAction;
class UStaminaComponent;
class UCombatComponent;
class UHealthComponent;
class UTraceHitboxComponent;
class UTargetingComponent;
class UTeamComponent;
class UStatusEffectComponent;
class UInventoryComponent;
class UWukongAnimInstance;
class UPlayerHUDWidget;
class ANPCCharacter;
class UInteractionPromptWidget;
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


UCLASS()
class BLACKMYTH_API AWukongCharacter : public ABlackMythCharacter
{
	GENERATED_BODY()

	friend class UWukongAnimInstance;

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
	virtual void Landed(const FHitResult& Hit) override;

	// 重写 TakeDamage 以支持通用伤害系统 (如 ApplyDamage)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// ========== 公共接口 ==========
	
	/** 受到伤害（委托给HealthComponent） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveDamage(float Damage, AActor* DamageInstigator);

	/** 设置无敌状态（委托给HealthComponent） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvincible(bool bInInvincible);

	/** 获取生命组件 */
	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	/** 获取当前状态 */
	UFUNCTION(BlueprintPure, Category = "State")
	EWukongState GetCurrentState() const { return CurrentState; }

	/** 检查角色是否已死亡 */
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDead() const { return CurrentState == EWukongState::Dead; }

	// ========== 动画访问器 ==========
	
	/** 获取当前连击索引 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetComboIndex() const;

	/** 计算最终伤害值（委托给CombatComponent） */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float CalculateDamage(bool bIsHeavyAttack = false, bool bIsAirAttack = false, int32 ComboIndex = 0) const;

	/** 获取基础攻击力 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetBaseAttackPower() const;

	/** 获取战斗组件 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	/** 获取目标锁定组件 */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	UTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }

	/** 设置 HUD Widget 引用 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetPlayerHUD(UPlayerHUDWidget* InHUD) { PlayerHUD = InHUD; }

	/** 获取 HUD Widget */
	UFUNCTION(BlueprintPure, Category = "UI")
	UPlayerHUDWidget* GetPlayerHUD() const { return PlayerHUD; }

	/** 是否正在冲刺 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	/** 是否正在对话中（禁用除E外的所有输入） */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	/** 设置对话状态（由DialogueComponent调用） */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetInDialogue(bool bInDialogue);

	/** 获取移动速度 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetMovementSpeed() const;

	/** 获取移动方向 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetMovementDirection() const;

	// 生命值变化委托已迁移到 UHealthComponent

	/** 获取体力组件 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

protected:
	// ========== 输入动作 ==========
	
	/** 翻滚输入动作 (Ctrl键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;

	/** 轻击输入动作 (鼠标左键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	/** 重击输入动作 (鼠标右键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HeavyAttackAction;

	/** 冲刺输入动作 (Shift键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	/** 战技/立棍输入动作 (X键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PoleStanceAction;

	/** 棍花输入动作 (V键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> StaffSpinAction;

	/** 使用道具输入动作 (R键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UseItemAction;

	/** 锁定目标输入动作 (鼠标中键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LockOnAction;

	/** 切换目标输入动作 (鼠标滚轮) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SwitchTargetAction;

	/** 影分身输入动作 (F键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShadowCloneAction;

	/** 定身术输入动作 (按键2) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FreezeSpellAction;

	/** 变身术输入动作 (按键3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TransformAction;

	/** 交互输入动作 (E键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	/** 背包开关输入动作 (Tab键) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleInventoryAction;

	// ========== 组件 ==========

	/** 生命值组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** 体力值组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	/** 战斗属性组件（管理攻击力、伤害倍率等） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** 武器 Hitbox 组件（用于攻击判定） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTraceHitboxComponent> WeaponTraceHitbox;

	/** 目标锁定组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTargetingComponent> TargetingComponent;

	/** 阵营组件（用于敌我识别） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTeamComponent> TeamComponent;

	/** 状态效果组件（管理中毒、减速等状态） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatusEffectComponent> StatusEffectComponent;

	/** 背包组件（管理物品和消耗品） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** 获取状态效果组件 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	UStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }

	/** 获取背包组件 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

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
	
	// ========== 攻击属性（时间相关，保留在角色类中） ==========

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
public:
	/** 攻击蒙太奇1（连击第1段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage1;

	/** 攻击蒙太奇2（连击第2段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage2;

	/** 攻击蒙太奇3（连击第3段） */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage3;

protected:
	/** 重击蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	/** 空中轻击蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AirLightAttackMontage;

	/** 棍花蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Skill")
	TObjectPtr<UAnimMontage> StaffSpinMontage;

	/** 立棍法蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Skill")
	TObjectPtr<UAnimMontage> PoleStanceMontage;

	/** 喝药蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Item")
	TObjectPtr<UAnimMontage> DrinkGourdMontage;

	/** 跳跃蒙太奇 (Start -> Loop -> Land) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimMontage> JumpMontage;

	/** 翻滚蒙太奇 (前) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeFwdMontage;

	/** 翻滚蒙太奇 (后) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeBwdMontage;

	/** 翻滚蒙太奇 (左) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeLeftMontage;

	/** 翻滚蒙太奇 (右) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Movement")
	TObjectPtr<UAnimMontage> DodgeRightMontage;

	// ========== 移动动画序列 ==========
	
	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> IdleAnimation;

	/** 行走动画 - 前进 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkForwardAnimation;

	/** 行走动画 - 后退 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkBackwardAnimation;

	/** 行走动画 - 左移 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkLeftAnimation;

	/** 行走动画 - 右移 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> WalkRightAnimation;

	/** 冲刺动画 - 前进 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintForwardAnimation;

	/** 冲刺动画 - 后退 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintBackwardAnimation;

	/** 冲刺动画 - 左移 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintLeftAnimation;

	/** 冲刺动画 - 右移 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Locomotion")
	TObjectPtr<UAnimSequence> SprintRightAnimation;

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

	// ========== 影分身配置 ==========

	/** 分身类（在蓝图中设置为 BP_WukongClone） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShadowClone")
	TSubclassOf<class AWukongClone> CloneClass;

	/** 影分身蒙太奇（召唤动画） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShadowClone")
	TObjectPtr<UAnimMontage> ShadowCloneMontage;

	/** 生成的分身数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShadowClone")
	int32 CloneCount = 2;

	/** 分身存活时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShadowClone")
	float CloneLifetime = 20.0f;

	/** 分身生成距离（距离玩家多远） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShadowClone")
	float CloneSpawnDistance = 150.0f;

	/** 影分身冷却时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShadowClone")
	float ShadowCloneCooldown = 30.0f;

	// ========== 定身术配置 ==========

	/** 定身术持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreezeSpell")
	float FreezeSpellDuration = 5.0f;

	/** 定身术冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FreezeSpell")
	float FreezeSpellCooldown = 15.0f;

	/** 定身术施放动画蒙太奇（可选） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreezeSpell")
	TObjectPtr<UAnimMontage> FreezeSpellMontage;

	/** 执行攻击 */
	void Attack();

	// ========== UI 配置 ==========

	/** 玩家 HUD Widget 类（在蓝图中设置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDClass;

private:
	// ========== UI 引用 ==========
	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUD;  // 玩家 HUD Widget 实例

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
	float LastAttackTime = 0.0f;       // 上次攻击时间
	int32 ComboCount = 0;              // 当前连击段数
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

	/** 生命值耗尽时的回调 */
	UFUNCTION()
	void OnHealthDepleted(AActor* Killer);

	/** 对敌人造成伤害时的回调（用于更新连击 UI） */
	UFUNCTION()
	void OnDamageDealtToEnemy(float Damage, AActor* Target, bool bIsCritical);

	// 连击计数（用于 UI 显示）
	int32 HitComboCount = 0;
	FTimerHandle ComboResetTimerHandle;
	void ResetHitCombo();

	// ========== 战技状态 ==========
	bool bIsUsingAbility = false;  // 是否正在使用战技
	float AbilityTimer = 0.0f;     // 战技计时器

	// ========== 背包状态 ==========
	bool bIsInventoryOpen = false;  // 背包是否打开

	// ========== 输入处理函数 ==========
	void OnDodgePressed();    // 翻滚按下
	void OnAttackPressed();   // 攻击按下
	void OnSprintStarted();   // 冲刺开始
	void OnSprintStopped();   // 冲刺结束
	void OnAbilityPressed();  // 战技按下
	void OnLockOnPressed();   // 锁定目标按下
	void OnSwitchTarget(const FInputActionValue& Value);  // 切换目标
	void ToggleInventory();   // 切换背包显示

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
	void PerformHeavyAttack();  // 执行重击
	void PerformStaffSpin();    // 执行棍花
	void PerformPoleStance();   // 执行立棍
	void PerformShadowClone();  // 执行影分身
	void PerformFreezeSpell();  // 执行定身术
	void UseItem();             // 使用物品
	void ResetCombo();          // 重置连击
	void ProcessInputBuffer();  // 处理输入缓冲

	/** 播放蒙太奇辅助函数 */
	void PlayMontage(UAnimMontage* MontageToPlay, FName SectionName = NAME_None);

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
	void UpdateFacingTarget(float DeltaTime);  // 锁定时更新角色朝向

	/**
	 * 从动画序列动态创建并播放蒙太奇
	 * 
	 * @param AnimSequence 要播放的动画序列
	 * @param SlotName 使用的 Slot 名称（默认 DefaultSlot）
	 * @param PlayRate 播放速率
	 * @return 蒙太奇时长，失败返回0
	 */
	float PlayAnimationAsMontageDynamic(UAnimSequence* AnimSequence, FName SlotName = FName("DefaultSlot"), float PlayRate = 1.0f);

	// ========== 音效系统 ==========
protected:
	/** 脚步声（走路和疾跑共用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> FootstepSound;

	/** 走路脚步声音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float WalkFootstepVolume = 0.2f;

	/** 疾跑脚步声音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float SprintFootstepVolume = 0.4f;

	/** 攻击音效（所有攻击动作共用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AttackSound;

	/** 攻击音效音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AttackSoundVolume = 1.0f;

	/** 闪避音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DodgeSound;

	/** 闪避音效音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float DodgeSoundVolume = 0.7f;

	/** 跳跃音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> JumpSound;

	/** 跳跃音效音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float JumpSoundVolume = 0.6f;

public:
	/** 播放脚步声（由动画通知调用） */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayFootstepSound();

	/** 播放攻击音效（由动画通知调用） */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayAttackSound();

	/** 播放闪避音效 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayDodgeSound();

	/** 播放跳跃音效 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayJumpSound();

	// ========== NPC交互系统 ==========
protected:
	/** 交互距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 300.0f;

	/** 检测频率（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionCheckInterval = 0.2f;

	/** 交互提示Widget类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|UI")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

	/** 当前可交互的NPC */
	UPROPERTY()
	ANPCCharacter* NearbyNPC;

	/** 交互提示UI实例 */
	UPROPERTY()
	UInteractionPromptWidget* InteractionPromptWidget;

	/** 检测计时器 */
	float InteractionCheckTimer;

	/** 对话状态标志 - 对话中禁用除E键外的所有操作 */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bIsInDialogue;

	/** 当前对话的NPC（用于距离检测） */
	UPROPERTY()
	ANPCCharacter* CurrentDialogueNPC;

	/** 对话自动结束的最大距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float DialogueBreakDistance = 500.0f;

	/** 检测附近的NPC */
	void CheckForNearbyNPC();

	/** 检测对话距离，超出则自动结束 */
	void CheckDialogueDistance();

	/** 显示交互提示 */
	void ShowInteractionPrompt();

	/** 隐藏交互提示 */
	void HideInteractionPrompt();

	/** 处理交互输入 */
	void OnInteract();

	// ========== 变身术系统 ==========
protected:
	/** 是否处于变身状态 */
	UPROPERTY(BlueprintReadOnly, Category = "Transform")
	bool bIsTransformed;

	/** 变身冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	float TransformCooldown = 60.0f;

	/** 变身持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	float TransformDuration = 30.0f;

	/** 蝴蝶Pawn类（在蓝图中配置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	TSubclassOf<APawn> ButterflyPawnClass;

	/** 当前生成的蝴蝶Pawn实例 */
	UPROPERTY()
	APawn* ButterflyPawnInstance;

	/** 变身前悟空的位置（用于将悟空移到地下隐藏） */
	FVector PreTransformLocation;

	/** 执行变身术 */
	void PerformTransform();

	/** 变身为蝴蝶 */
	void TransformToButterfly();

	/** 变身结束计时器回调 */
	void OnTransformDurationEnd();

	/** 变身结束计时器Handle */
	FTimerHandle TransformTimerHandle;

	/** 清除所有敌人的仇恨 */
	void ClearAllEnemyAggro();

public:
	/** 变回悟空（public以便ButterflyPawn调用） */
	void TransformBackToWukong();

	/** 是否处于变身状态 */
	UFUNCTION(BlueprintPure, Category = "Transform")
	bool IsTransformed() const { return bIsTransformed; }

	/** 获取变身冷却剩余时间 */
	UFUNCTION(BlueprintPure, Category = "Transform")
	float GetTransformCooldownRemaining() const;
};
