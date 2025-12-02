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

	// Hit Stun Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HitStunDuration = 0.5f;

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

	// State Methods
	void ChangeState(EWukongState NewState);
	void UpdateState(float DeltaTime);
	void UpdateIdleState(float DeltaTime);
	void UpdateMovingState(float DeltaTime);
	void UpdateAttackingState(float DeltaTime);
	void UpdateDodgingState(float DeltaTime);
	void UpdateHitStunState(float DeltaTime);
	void UpdateDeadState(float DeltaTime);

	// Combat Methods
	void PerformAttack();
	void ResetCombo();
	void ProcessInputBuffer();

	// Dodge Methods
	void PerformDodge();
	void UpdateDodgeMovement(float DeltaTime);

	// Cooldown Management
	bool IsCooldownActive(const FString& CooldownName) const;
	void StartCooldown(const FString& CooldownName, float Duration);
	void UpdateCooldowns(float DeltaTime);

	// Helper Methods
	void Die();
	void UpdateMovementSpeed();
	FVector GetMovementInputDirection() const;
};
