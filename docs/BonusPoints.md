# 项目加分项说明文档

本文档旨在补充说明项目在版本控制、代码质量、C++特性使用、架构设计及用户体验等方面的亮点，以供评分参考。

## 1. 版本控制与协作方面

本项目采用 Git 进行全流程版本控制，并遵循规范的协作流程（详见 `docs/cooperatingSpecification.md`）：

*   **Git LFS 支持**：针对虚幻引擎项目的大资产特性，配置了 `.gitattributes` 文件，使用 Git LFS (Large File Storage) 管理 `.uasset`、`.umap` 等二进制大文件，确保仓库体积可控且拉取高效。
*   **严格的分支策略**：
    *   **`main`**：仅在关键节点更新的稳定分支。
    *   **`dev`**：开发集成分支，禁止直接 Commit。
    *   **其余的独立分支**：功能开发与 Bug 修复均在独立分支进行，完成后合并回 `dev`。
*   **规范的开发流**：
    *   采用 `git fetch` + `git rebase` （或者是`git pull --rebase`）替代 `git pull`，保持提交历史的线性整洁。
    *   遵循“一人负责一文件”的原则（尽可能），最大程度减少二进制资产的合并冲突。
*   **清晰的提交历史**：Commit Message 遵循约定式提交规范（如 `[feat]`, `[fix]`, `[refactor]`），清晰记录了每个功能的开发历程。

## 2. 代码质量

代码编写遵循虚幻引擎官方编码标准，具有良好的可读性和可维护性：

*   **命名规范**：严格遵守 UE 命名约定，如类名以 `A` 或 `U` 开头（`AWukongCharacter`, `UHealthComponent`），布尔变量以 `b` 开头（`bIsInvincible`），枚举使用 `E` 前缀（`EEnemyState`）。
*   **注释详尽**：核心逻辑均配有详细的中文注释。例如在 `WukongCharacter.cpp` 中，对 `BeginPlay` 中的移动组件配置、`Tick` 中的状态机更新均有清晰说明。
*   **日志系统**：合理使用 `UE_LOG` 进行运行时调试与状态追踪，区分 `Log`、`Warning` 和 `Error` 级别，便于快速定位问题。
*   **模块化设计**：将功能拆分为独立的组件（Component），避免了 "God Class"（上帝类）的出现。例如，生命值管理逻辑封装在 `HealthComponent`，背包逻辑封装在 `InventoryComponent`。

## 3. 使用了多种C++11及以上的特性

项目中广泛应用了现代 C++ 特性，提升了代码的安全性和效率：

*   **`auto` 类型推导**：在遍历容器时使用 `auto` 简化代码并避免类型错误。
    *   *示例 (`WukongCharacter.cpp`)*:
        ```cpp
        // 遍历冷却表，自动推导迭代器类型
        for (auto& Cooldown : CooldownMap)
        ```
*   **`enum class` (强类型枚举)**：使用 `enum class` 替代传统 `enum`，避免命名空间污染和隐式类型转换，增强类型安全。
    *   *示例 (`EnemyBase.h`)*:
        ```cpp
        UENUM(BlueprintType)
        enum class EEnemyState : uint8
        {
            EES_Patrolling UMETA(DisplayName = "Patrolling"),
            EES_Chasing    UMETA(DisplayName = "Chasing"),
            // ...
        };
        ```
*   **`override` 关键字**：在派生类重写虚函数时显式使用 `override`，确保正确覆盖基类函数，防止签名不匹配导致的潜在 Bug。
    *   *示例 (`EnemyBase.h`)*:
        ```cpp
        virtual void BeginPlay() override;
        virtual void Tick(float DeltaTime) override;
        ```
*   **Lambda 表达式与委托**：在定时器（TimerManager）或事件绑定中使用委托，实现异步逻辑处理。
*   **智能指针与对象管理**：利用 UE 的反射系统和垃圾回收机制（`UPROPERTY`），结合 C++ 指针安全检查（如 `if (Object)`），确保内存安全。

## 4. 架构设计优秀

项目架构设计充分利用了虚幻引擎的 Gameplay 框架，结构清晰且易于扩展：

*   **Actor-Component 模式**：
    *   核心逻辑被解耦为多个组件。`AWukongCharacter` 作为宿主，挂载了 `UHealthComponent`（生命值）、`UStaminaComponent`（体力）、`UCombatComponent`（战斗核心）、`UInventoryComponent`（背包）等。
    *   这种设计使得组件可以在不同角色（如玩家和敌人）之间复用。例如，`UHealthComponent` 同时被 `AWukongCharacter` 和 `AEnemyBase` 使用。
*   **MVC 模式在 UI 中的应用**：
    *   **Model (数据层)**：`WalletComponent`、`StatusEffectComponent` 持有数据。
    *   **View (视图层)**：`UPlayerHUDWidget`、`UInventoryBarWidget` 负责显示。
    *   **Controller (控制层)**：通过动态多播委托（Dynamic Multicast Delegates）实现数据驱动更新。当数据变化时（如 `OnGoldChanged`），自动通知 UI 刷新，无需 UI 每帧轮询数据。
*   **接口驱动设计**：
    *   通过接口（如伤害接收接口）统一处理不同对象的交互，降低了模块间的耦合度。
*   **行为树 (Behavior Tree) AI 系统**：
    *   敌人的 AI 逻辑（巡逻、追逐、攻击）通过行为树实现，而非硬编码在 `Tick` 函数中，使得 AI 行为更智能、易调试且易扩展。

## 5. 目录结构清晰

项目文件结构组织严谨，便于资产管理和代码查找：

*   **Source 目录**：C++ 源码按模块分类，`BlackMyth/` 下包含 `Components/`（组件）、`UI/`（界面）、`Items/`（物品）、`Combat/`（战斗）等子目录。
*   **Content 目录**：资产按类型和用途分类，如 `Blueprints/`（蓝图）、`Characters/`（角色）、`UI/`（界面资产）、`Maps/`（关卡），避免了资产混乱堆放。

## 6. 界面与体验

注重用户交互体验的细节打磨：

*   **动态 UI 反馈**：血条、体力条、金币数量均能实时响应游戏内变化。
*   **交互提示**：实现了 `InteractionPromptWidget`，当玩家靠近可交互对象（如 NPC 或掉落物）时，自动显示按键提示，提升操作引导性。
*   **平滑的摄像机控制**：在 `WukongCharacter` 中配置了 `SpringArmComponent` 和 `CameraComponent`，实现了平滑的跟随视角，并处理了遮挡和距离限制。
