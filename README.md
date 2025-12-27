# BlackMyth_Wukong——仿知名游戏黑神话悟空的课程大作业

**不要直接git clone！！！有需要的部分可以自行选取下载，直接git clone会消耗大量git LFS带宽，也可以到release去下载**

## 项目概述

一个基于UE5.4的第三人称动作游戏，围绕战斗体验、敌人与 Boss 行为、场景与交互系统、物品与状态效果、土地庙交互与传送、商店与金币、存档读档等核心要素迭代开发。文档按功能模块梳理当前实现与技术细节。本文将功能与技术要点分区说明，并在最后提供简要的项目分工表。

## 主要功能

- 开始菜单与暂停菜单：基础 UI 流程可用，含开始、退出、设置/存档入口。
- 战斗系统：包含定身术、立棍 AOE、硬直与韧性条、友方伤害屏蔽、相机/移动修复等。
- 敌人与 Boss：普通敌人（近战、斧头兵、远程兵）与 Boss「二郎神」的招式、闪避、召唤哮天犬等行为；完善的受击/死亡动画与音效。
- Temple（土地庙）系统：交互面板、传送（支持指定点与地图分布）、回复与复活点（重生），并与交易/商店联动。
- 物品与背包：支持背包组件（按 `I` 开关）、药瓶类与治疗 Buff、装备使用与状态图标，键位冲突梳理（技能键与装备键分离）。
- 状态效果系统（Buff/Debuff）：中毒、减速等状态带来 UI 图标、倒计时与角色颜色变化；在中毒下禁用部分动作。
- 商店与金币：敌人死亡掉落金币、按 `F` 拾取、金币 HUD 显示、商品管理器与商店 UI；拾取逻辑优先级与资源补全。
- NPC 对话与剧情：CSV 数据驱动的对话系统、UI 组件、摄像机切换；与 Boss 开场动画、锁定/交互逻辑兼容。
- 场景与地图：探索/普通战斗/Boss 站场景切换（含 BGM 绑定）、新增地图与 Boss 区域、传送点分布与地形适配。
- 存档与读档：血量/体力、敌人状态与位置的存档读档，读档流程直接跳转到地图；命名存档与面板 UI。
- 音效：角色移动/攻击/翻滚音效、Boss 攻击命中音效。

## 系统模块与技术细节

### 1. 战斗与技能系统
战斗系统是游戏的核心体验。
- **攻击判定**：通过 `UTraceHitboxComponent` 配合动画通知 (`AnimNotify_ActivateHitbox`) 实现精确的武器碰撞检测。支持多段连招，通过 `OpenComboWindow` 和 `CloseComboWindow` 通知管理输入缓冲。
- **技能体系**：
  - **定身术**：锁定目标后施加定身状态，敌人模型材质高亮（金色），并停止行为树逻辑。
  - **立棍 AOE**：实现范围伤害检测，修复了施法期间的相机跟随与角色移动锁定问题。
  - **变身术**：实现了变身蝴蝶的机制，包含独立的 `ButterflyPawn`、输入映射切换及 CD 管理。
- **数值与状态**：
  - **硬直**：引入韧性条机制，韧性耗尽时触发受击动画，打断当前行为。
  - **体力限制**：攻击、翻滚、疾跑均消耗体力，体力不足时限制特定动作。
  - **友伤屏蔽**：基于 `ETeam` 枚举区分阵营，防止同阵营误伤。

### 2. 敌人 AI 与 Boss 战
敌人 AI 基于 **行为树 (Behavior Tree)**构建，实现了分层决策。
- **普通敌人**：
  - **感知系统**：利用 `UPawnSensingComponent` 视觉与听觉感知玩家。
  - **行为模式**：实现了巡逻 (`BTT_FindRandomPatrol`)、追击、攻击 (`BTT_Attack`) 三种状态的流转。
  - **兵种差异**：近战兵（普通/斧头）侧重近身压制，远程兵 (`ARangedEnemy`) 保持距离并发射投射物。
- **Boss 二郎神**：
  - **多阶段战斗**：根据血量触发阶段转换，二阶段解锁新技能与特效。
  - **技能组合**：包含轻重击连招、闪避反击、以及召唤哮天犬 (`XiaoTian`) 进行协同攻击。
  - **场景互动**：通过 `BossCombatTrigger` 划定战斗区域，触发空气墙防止玩家脱战，并切换战斗 BGM。

### 3. 土地庙系统
土地庙作为游戏的核心交互枢纽，集成了多种功能模块。
- **交互框架**：基于 `IInteractInterface` 接口，玩家进入范围后显示按键提示 (`Q`)，触发 `TempleMenuWidget`。
- **传送网络**：
  - 支持预设点传送与地图任意点传送。
  - 解决了传送后的地形高度适配问题，防止角色卡入地底。
- **功能集成**：
  - **休息**：回复满血量与体力，重置场景怪物刷新。
  - **重生点**：激活土地庙后自动记录为重生点 (`Respawn Point`)。
  - **商店入口**：直接关联交易界面，购买消耗品与强化道具。

### 4. 物品与背包系统
- **背包管理**：
  - `UInventoryComponent` 维护物品数据，支持物品的添加、移除与使用。
  - 实现了快捷栏映射，按 `I` 键打开背包时，数字键 `1-4` 用于选择装备；关闭背包时恢复为技能释放键，解决了键位冲突。
- **物品类型**：
  - **消耗品**：如药瓶 (`PotionActor`)，使用时播放喝药动画并挂载模型。
  - **增益道具**：如怒火丹（攻击 Buff）、金刚丹（防御 Buff），使用后通过 `UStatusEffectComponent` 施加效果。

### 5. 状态效果系统
采用组件化设计，`UStatusEffectComponent` 负责管理所有挂载在角色身上的效果。
- **架构设计**：`UStatusEffectBase` 作为基类，定义了 `OnApply`, `OnTick`, `OnRemove` 生命周期。
- **具体效果**：
  - **中毒**：持续扣血，屏幕边缘泛绿，并禁用疾跑等高体力动作。
  - **减速**：修改 `UCharacterMovementComponent` 的 `MaxWalkSpeed`。
  - **UI 表现**：状态图标动态显示在 HUD 上，包含倒计时遮罩与层数显示。

### 6. 经济与商店系统
- **金币循环**：
  - **产出**：敌人死亡掉落 `AGoldPickup` Actor。
  - **拾取**：支持按 `F` 键吸附拾取，金币飞向 HUD 金币栏的动画轨迹经过贝塞尔曲线平滑处理。
- **交易系统**：
  - `ShopManager` 单例类管理商品列表 (`FShopItemData`)。
  - 商店 UI 支持商品预览、价格检查与购买确认，购买成功后自动扣除金币并存入背包。

### 7. 剧情与交互系统
- **对话系统**：
  - **数据驱动**：使用 CSV 表格导入对话数据 (`FDialogueEntry`)，包含文本、说话人、时长等信息。
  - **演出控制**：对话开始时自动切换摄像机视角（特写/过肩），对话结束后恢复。
  - **状态约束**：对话期间禁用玩家移动与攻击输入，防止穿模或逻辑中断。

### 8. 场景与存档
- **场景管理**：
  - 实现了探索、普通战斗、Boss 战三种场景状态的无缝切换，自动淡入淡出背景音乐 (`BGM`)。
- **持久化数据**：
  - `UBlackMythSaveGame` 继承自 `USaveGame`，序列化玩家属性（血量、位置、金币）、背包数据及世界状态（Boss 击杀标记）。
  - 支持多存档槽位，读档时通过 `UGameplayStatics::OpenLevel` 重新加载地图并恢复状态。

## 项目架构与类设计

项目采用 Unreal Engine 典型的 **Actor-Component** 架构，结合 **MVC** 思想进行 UI 与逻辑分离，确保了代码的高内聚低耦合。

### 1. 核心类层次结构

项目基于 UE 的反射机制构建了清晰的继承体系，主要分为角色、物品、状态效果三大分支：

*   **角色体系**
    *   `ACharacter` (UE基类)
        *   `ABlackMythCharacter`: 项目角色基类，封装通用摄像机 (`SpringArm`, `Camera`)、输入映射上下文 (`InputMappingContext`) 及基础移动逻辑。
            *   `AWukongCharacter`: 玩家控制的主角。实现了复杂的战斗状态机 (`EWukongState`)、技能系统（定身、变身、分身）、背包交互接口及与 NPC/场景物体的交互逻辑。
            *   `ANPCCharacter`: 中立 NPC 基类。挂载 `UDialogueComponent`，处理对话数据加载与交互触发。
            *   `AEnemyBase`: 敌人基类。集成 AI 控制器接口、感知组件 (`PawnSensing`)、属性组件 (`Health`, `Stamina`) 及受击/死亡流程。
                *   `ARegularEnemy`: 普通近战敌人，实现基础巡逻与近身攻击。
                *   `ARangedEnemy`: 远程敌人，扩展了投射物生成与远程 AI 行为。
                *   `ABossEnemy`: Boss（如二郎神）。扩展了多阶段战斗 (`EBossPhase`)、霸体/韧性机制、特定技能（召唤哮天犬）及过场动画触发器。

*   **物品与交互体系**
    *   `AActor` (UE基类)
        *   `AInteractableActor`: 可交互物体基类。实现了 `IInteractInterface` 接口，用于土地庙 (`Temple`) 等场景设施，支持交互范围检测与 UI 提示。
        *   `APotionActor`: 药瓶可视化 Actor，用于喝药动画中的模型挂载。
        *   `AGoldPickup`: 掉落物基类。实现了自动吸附/按键拾取逻辑与 HUD 动画。

*   **状态效果体系 (Status Effects)**
    *   `UObject` (UE基类)
        *   `UStatusEffectBase`: 状态效果抽象基类。定义了 `ApplyEffect`, `RemoveEffect`, `TickEffect` 等虚接口。
            *   `UPoisonEffect`: 中毒效果，持续扣血并禁用部分动作。
            *   `USlowEffect`: 减速效果，修改角色移动速度属性。
            *   `UAttackBuffEffect` / `UDefenseBuffEffect`: 数值增益效果。

### 2. 组件化设计

为了降低耦合，核心功能被封装为组件 (`UActorComponent`)，可复用于不同角色：

*   **战斗核心**：
    *   `UCombatComponent`: 处理攻击判定框 (`Hitbox`) 管理、连招窗口 (`Combo Window`) 记录及攻击状态流转。
    *   `UTraceHitboxComponent`: 基于动画通知 (`AnimNotify`) 的精确武器碰撞检测组件。
*   **属性管理**：
    *   `UHealthComponent`: 管理生命值、无敌帧及死亡回调。
    *   `UStaminaComponent`: 管理体力值消耗与恢复（奔跑、翻滚、攻击）。
*   **系统功能**：
    *   `UInventoryComponent`: 背包组件，管理 `FItemData` 列表、快捷栏映射及物品使用逻辑。
    *   `UDialogueComponent`: 对话组件，解析 CSV 数据表，管理对话索引与 UI 显示。

### 3. 模块间关系

*   **逻辑与表现分离 (MVC)**：
    *   **Model**: `AWukongCharacter` 等类维护核心数据（血量、金币、背包）。
    *   **View**: `UPlayerHUDWidget`, `UInventoryBarWidget` 等 UMG 控件负责显示。
    *   **Controller/Bridge**: 通过多播委托 (`OnHealthChanged`, `OnGoldChanged`) 通信。角色状态变化时广播事件，UI 监听并更新，互不持有引用。
*   **AI 决策系统**：
    *   `AEnemyAIController` 持有行为树 (`Behavior Tree`) 和黑板 (`Blackboard`)。
    *   `AEnemyBase` 作为执行者，提供 `Attack()`, `MoveTo()` 等原子操作供行为树节点调用。
    *   感知组件 (`AIPerception`) 收集环境信息写入黑板，驱动行为树分支切换（巡逻 -> 追击 -> 攻击）。

## C++ 特性应用

项目代码深度结合了现代 C++ 特性与 UE 框架机制，以下是典型的应用场景：

1. **容器与迭代器**
   
   - **说明**：大量使用 UE 的泛型容器 `TArray<T>` 和 `TMap<K, V>` 替代原生数组，配合 C++11 基于范围的 `for` 循环提高代码可读性与安全性。
   - **代码示例**：
     ```cpp
     // WukongCharacter.cpp: 遍历范围内的掉落物进行吸附
     TArray<AGoldPickup*> NearbyGolds;
     // ... (填充数组逻辑)
     for (AGoldPickup* Gold : NearbyGolds)
     {
         if (Gold && !Gold->IsPendingKill())
         {
             Gold->StartMagnetMovement(this);
         }
     }
     ```
   
2. **多态与虚函数**
   
   - **说明**：利用虚函数实现不同角色的差异化行为。基类定义接口，子类通过 `override` 关键字实现具体逻辑。
   - **代码示例**：
     ```cpp
     // EnemyBase.h
     virtual void TakeDamage(float DamageAmount) override;
     
     // BossEnemy.cpp: Boss 受到伤害时可能有减伤或阶段转换逻辑
     void ABossEnemy::TakeDamage(float DamageAmount)
     {
         // 调用父类基础逻辑
         Super::TakeDamage(DamageAmount);
         
         // Boss 特有的阶段转换检查
         if (HealthComponent->GetHealthPercent() < 0.5f && CurrentPhase == EBossPhase::PhaseOne)
         {
             EnterPhaseTwo();
         }
     }
     ```
   
3. **模板 **
   
   - **说明**：主要用于类型安全的动态转换与资源加载。UE 的 `Cast<T>` 是最常用的模板函数，用于运行时类型检查。
   - **代码示例**：
     ```cpp
     // WukongCharacter.cpp: 碰撞检测中的类型判断
     void AWukongCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, ...)
     {
         // 使用模板函数 Cast 安全地将 AActor* 转换为 ANPCCharacter*
         if (ANPCCharacter* NPC = Cast<ANPCCharacter>(OtherActor))
         {
             CurrentDialogueNPC = NPC;
             InteractionPromptWidget->SetVisibility(ESlateVisibility::Visible);
         }
     }
     ```
   
4. **C++11 新特性**
   - **说明**：广泛使用 `enum class` (强类型枚举)、`nullptr` (空指针常量) 和 `auto` (自动类型推导) 增强代码健壮性。
   - **代码示例**：
     ```cpp
     // WukongCharacter.h: 强类型枚举定义角色状态
     enum class EWukongState : uint8
     {
         Idle,
         Moving,
         Attacking,
         HitStun,
         Dead
     };
     
     // WukongCharacter.cpp: 使用 nullptr 初始化与 auto 推导
     AActor* CurrentTarget = nullptr; 
     
     // 遍历冷却表，auto 自动推导为 TPair<FString, float>&
     for (auto& CooldownPair : CooldownMap)
     {
         CooldownPair.Value -= DeltaTime;
     }
     ```

## 工程结构（摘要）

- `Source/`：目标与编辑器 Target、C++ 源码（如 `BlackMyth.Target.cs`、`BlackMythEditor.Target.cs`、项目模块目录）。
- `Content/`：美术与蓝图资源，含 CombatSystem、Characters、UI、Levels、Temple、InfinityBlade 等子目录。
- `Config/`：默认引擎/编辑器/游戏配置。
- `Plugins/`：如 VisualStudioTools 插件与其 Binaries/Config/Docs。
- `Binaries/`、`Intermediate/`、`Saved/`：构建产物、中间文件与保存数据。
- `BlackMyth.uproject`：UE 项目文件。

## 提交树概览

- 分支：`dev` 为主开发分支；特性分支如 `shop`、`map`、`boss`、`npc`、`temple` 等按主题合并至 `dev`。
- 里程碑：
    - NPC 对话系统与 CSV 数据表导入；
    - 战斗系统：定身术、立棍 AOE、友伤屏蔽、相机/移动修复；
    - 敌人体系：近战/斧头/远程三类武器与行为；
    - Boss：二郎神的招式、闪避、召唤哮天犬与开场演出；
    - Temple：传送/交易/复活点与 UI 面板；
    - 背包/物品：装备使用、药瓶与 Buff UI、键位冲突修正；
    - 状态效果系统：中毒与减速；
    - 商店与金币：掉落/拾取/HUD、商品管理器与商店 UI；
    - 场景切换与地图更新（含 BGM）；
    - 存档读档：血量/体力与敌人状态；
    - 音效与字体资源、编码与注释规范修复。

## 构建与运行

1. 打开 `BlackMyth.uproject`（建议用 Unreal Editor 对应版本）。
2. 首次打开等待资源与编译完成；如需打包，按项目设置选择平台（Win64 等）。
3. 运行地图与关卡：可在 `Content/` 下选择演示关卡或 Boss 区域（如 L_showcase 或 Boss 战场景）。

## 代码规范与质量保障

- 注释与编码：注释与命名遵循更贴近 Google C++ 风格的规范化修订。
- 合并策略：主题分支合并至 `dev`，阶段性在合并前后进行修复与回归验证。
- 资源管理：UI 字体/音效/动画资产按模块归档；触发与绑定通过蓝图与 C++ 组合实现。

## 加分项

### 1. 版本控制与协作

- **GitHub 使用规范**：项目托管于 GitHub，采用标准的 Git Flow 工作流。设立 `dev` 为主开发分支，`shop`、`map`、`boss` 等为特性分支，开发完成后通过PR或者禁止fast-forward的merge来合并到dev分支，保证了主分支的稳定性。
- **Commit 记录清晰**：提交信息遵循 `[feat]`, `[fix]`, `[refactor]`, `[merge]` 等前缀规范，清晰记录了每个阶段的功能迭代与 Bug 修复，便于回溯与维护。
- **合理分工**：团队成员根据各自擅长的领域（战斗系统、AI 逻辑、UI 交互、资源管理）进行了明确的模块划分，避免了代码冲突，提高了开发效率。

### 2. 开发特性

- **C++11/14/17 特性使用丰富**：
    - **现代语法应用**：广泛使用 `auto` 类型推导、`enum class` 强类型枚举、`nullptr`、基于范围的 `for` 循环以及 Lambda 表达式（如在 UI 事件绑定中），提升了代码的可读性与安全性。
    - **模板与泛型**：熟练使用 UE 的模板函数（如 `Cast<T>`）进行类型安全的转换与资源获取。
- **优雅的架构设计**：
    - **组件化解耦**：将战斗、背包、状态效果封装为独立的 `UActorComponent`，实现了“组合优于继承”的设计原则，便于功能复用与扩展。
    - **MVC 模式实践**：严格分离数据模型（Character/Component）与视图表现（Widget），通过委托机制通信，降低了模块间的耦合度。
- **目录结构清晰**：
    - 源码 (`Source`)、资源 (`Content`)、配置 (`Config`) 分离。
    - C++ 类与蓝图资源按功能模块（Combat, UI, Items, AI）分类存放，便于索引与管理。

### 3. 界面与体验

- **界面精美**：
    - UI 设计风格统一，采用了霞鹜文楷中文字体，配合精心挑选的背景图与图标（如技能栏、背包栏），视觉体验和谐。
    - 实现了动态的 HUD 交互，如金币拾取的贝塞尔曲线飞行轨迹、状态效果的倒计时遮罩。
- **流畅动画**：
    - 角色动作（攻击、翻滚、喝药）与音效完美同步，通过 Animation Notify 精确触发。
    - 场景切换与 UI 面板（如土地庙、商店）的呼出/隐藏带有平滑的过渡效果。
- **游戏稳定性**：
    - 经过多轮测试与 Bug 修复（如修复空指针引用、数组越界、资源加载错误），保证了游戏运行的稳定性。
    - 针对地形与传送点进行了适配优化，解决了角色卡死或穿模的问题。

## 项目分工

| githubID    | 贡献度 | 
| ----------- | ------ |
| wendaining  | 25%    |
| Palind-Rome | 25%    |
| onnisama    | 25%    |
| Aphrody-Dy  | 25%    |

## 文档更新日期

2025年12月27日
