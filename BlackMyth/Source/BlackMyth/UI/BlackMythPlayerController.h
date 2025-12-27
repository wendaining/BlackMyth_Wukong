#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "BlackMythPlayerController.generated.h"

class UUserWidget;
struct FInputActionValue;

/**
 * 玩家控制器：负责输入映射和暂停菜单的切换。
 *
 * 说明：
 * - 构造函数在实现文件中使用初始化列表初始化成员。
 * - 重写的虚函数使用 `override` 标注。
 */
UCLASS()
class BLACKMYTH_API ABlackMythPlayerController : public APlayerController {
    GENERATED_BODY()

public:
    /** 构造函数。 */
    ABlackMythPlayerController();

    /**
     * 本地玩家使用的输入映射上下文（在编辑器中设置）。
     * UPROPERTY 保持原始访问性以便蓝图/编辑器使用。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* PlayerMappingContext;

    /** 用于切换暂停菜单的输入动作（在编辑器中设置）。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PauseAction;

    /** 继续游戏。 */
    void ContinueGame();
protected:
    /** 在 BeginPlay 中为本地玩家添加增强输入映射上下文。 */
    virtual void BeginPlay() override;

    /** 绑定输入组件（Enhanced Input）。 */
    virtual void SetupInputComponent() override;

    UFUNCTION()
    void EnterLoadGameFromPause();

    /** 暂停菜单的 Widget 类（在编辑器中设置）。 */
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuClass;

    /** 运行时创建的暂停菜单实例。 */
    UPROPERTY()
    UUserWidget* PauseMenuInstance;

    /** 当 PauseAction 触发时切换暂停菜单的显示状态。 */
    void TogglePauseMenu(const FInputActionValue& Value);

    UFUNCTION()
    void Interact();

private:
    /** 等待流式关卡加载完毕后再恢复角色移动。 */
    void BeginDeferredSpawnProtection();

    /** 检查流式关卡是否就绪，若就绪则恢复移动和碰撞。 */
    void TryEnablePawnAfterStreaming();

    /** 探测脚下地面，找到可行走平面并给出安全落点。 */
    bool ProbeGroundAndGetSafeLocation(class AWukongCharacter* Wukong, FVector& OutSafeLocation) const;

    FTimerHandle StreamingCheckHandle;
    float CachedGravityScale = 1.0f;
    TEnumAsByte<EMovementMode> CachedMovementMode = MOVE_Walking;
    bool bSpawnProtectionActive = false;

    /** 出生保护起始时间与上限，避免长时间悬浮。 */
    float SpawnProtectionStartTime = 0.f;
    float MaxSpawnProtectionDuration = 3.0f; // 秒

    /** 地面探测深度（单位厘米）。 */
    float GroundProbeDistance = 5000.0f;
};
