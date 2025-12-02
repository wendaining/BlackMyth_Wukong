# 《黑神话：悟空》期末项目执行文档

> **项目周期**：2024/12/02 - 2024/12/28（4周）  
> **引擎版本**：Unreal Engine 5.4 + Visual Studio 2022  
> **核心约束**：所有游戏逻辑必须用 C++ 实现，蓝图仅作数据资产配置

---

## 目录

1. [项目需求总结 (Scope)](#1-项目需求总结-scope)
2. [整体战略目标 (High-Level Strategy)](#2-整体战略目标-high-level-strategy)
3. [团队分工文档 (Roles & Responsibilities)](#3-团队分工文档-roles--responsibilities)
4. [短期与长期目标 (Timeline)](#4-短期与长期目标-timeline)
5. [技术避坑指南 (Survival Guide)](#5-技术避坑指南-survival-guide)
6. [附录：核心类骨架代码](#6-附录核心类骨架代码)

---

## 1. 项目需求总结 (Scope)

### 1.1 MVP 功能清单

#### 角色系统（必须实现）

| 功能 | 描述 | 验收标准 |
|-----|------|---------|
| 基础移动 | 行走、奔跑、跳跃 | 使用 CharacterMovement，响应输入流畅 |
| 闪避/翻滚 | 短距离位移 + 无敌帧 | 按键触发，有冷却时间，无敌帧内不受伤 |
| 基础攻击 | 至少 3 段连招 | 输入缓冲队列，可连续触发，可中断 |
| 状态切换 | Idle/Attack/HitStun/Dead | 状态机控制，状态间正确转换 |

#### 战斗系统（必须实现）

| 功能 | 描述 | 验收标准 |
|-----|------|---------|
| 生命值系统 | 玩家和敌人血量管理 | 受伤扣血，归零死亡 |
| Hitbox/Hurtbox | 攻击范围判定 | Trace 或碰撞组件检测命中 |
| 硬直反馈 | 受击时短暂禁止行动 | 受击后有明显停顿 |
| 技能系统 | 至少 1 个技能（棒击震地） | 范围伤害 + 冷却时间 |
| Boss 血条 | UI 显示 Boss 生命值 | 实时更新，C++ 驱动 |

#### 敌人与 Boss（必须实现）

| 功能 | 描述 | 验收标准 |
|-----|------|---------|
| 3 种小怪 | 近战、远程、机动型 | 继承自同一基类，行为不同 |
| 基础 AI | 追击、攻击、闪避 | C++ 实现状态机，无蓝图逻辑 |
| Boss | 1 个多阶段 Boss | 血量阈值触发阶段切换 |
| 怒气/阶段 | Boss 第二阶段增强 | 行为模式变化明显 |

#### 场景功能（必须实现）

| 功能 | 描述 | 验收标准 |
|-----|------|---------|
| 3D 场景 | 可移动的战斗区域 | 地形、障碍物基本完整 |
| 场景切换 | Trigger Volume 触发 | 进入区域后切换/加载场景 |
| 动态音乐 | 战斗/探索音乐切换 | C++ 控制 AudioComponent |

#### UI 功能（必须实现）

| 功能 | 描述 | 验收标准 |
|-----|------|---------|
| 标题菜单 | 开始/退出游戏 | Widget 布局，C++ 逻辑 |
| 玩家血条 | 实时显示生命值 | C++ 更新 ProgressBar |
| Boss 血条 | Boss 战时显示 | 进入战斗显示，击杀隐藏 |
| 技能冷却 | 显示技能剩余 CD | C++ 更新 |
| 死亡重开 | 死亡后重新开始 | 重置关卡状态 |
| 暂停菜单 | 暂停/继续/退出 | C++ 控制 GamePause |

### 1.2 C++ 技术要求（评分点）

由于 UE 不能直接使用 STL，需要使用 UE 等价容器来展示相同的编程能力：

| 技术点 | UE 实现方式 | 代码示例位置 |
|-------|------------|-------------|
| 容器 | `TArray`, `TMap`, `TSet` | 连招缓冲、冷却表、敌人列表 |
| 多态 | 抽象基类 + 虚函数重写 | `AEnemyBase` → 子类 |
| 模板 | 自定义模板类/函数 | `TDamageCalculator<TPolicy>` |
| C++11 Lambda | `TArray::RemoveAll` 等 | 过滤无效敌人 |
| 智能指针 | `TUniquePtr`, `TSharedPtr` | 非 UObject 对象管理 |
| 移动语义 | `MoveTemp()` | 高效数据转移 |
| 委托/回调 | `TFunction`, UE Delegate | 事件系统 |

---

## 2. 整体战略目标 (High-Level Strategy)

### 2.1 开发原则

```
┌─────────────────────────────────────────────────────────┐
│                    核心原则                              │
├─────────────────────────────────────────────────────────┤
│  1. 做"游戏"而不是"引擎" - 复用 UE 现有系统            │
│  2. C++ = 逻辑，蓝图 = 数据 - 严格分离                  │
│  3. 代码质量 > 画面效果 - 优先保证功能完整              │
│  4. 小步快跑 - 每周可演示的增量交付                     │
└─────────────────────────────────────────────────────────┘
```

### 2.2 工作流规范

#### 蓝图与 C++ 的职责划分

```
C++ 负责（必须）                    蓝图负责（允许）
─────────────────                  ─────────────────
• 输入处理逻辑                      • 模型/骨骼资产引用
• 状态机转换                        • 动画蒙太奇配置
• 攻击判定计算                      • 材质/特效引用
• AI 决策逻辑                       • 音效资源引用
• 伤害计算公式                      • UI Widget 布局
• 冷却时间管理                      • 参数默认值配置
• UI 数据更新                       • 关卡物体摆放
• 场景切换逻辑
```

#### 资产命名约定

```
蓝图类：       BP_<C++类名>          例：BP_WukongCharacter
动画蒙太奇：   AM_<角色>_<动作>      例：AM_Wukong_Attack01
动画通知：     AN_<功能>             例：AN_AttackHit, AN_ComboWindow
Widget：      WBP_<功能>            例：WBP_PlayerHUD
```

### 2.3 复用 UE 系统清单

| 需求 | 使用 UE 系统 | 不要自己写 |
|-----|-------------|-----------|
| 角色移动 | `UCharacterMovementComponent` | 物理模拟 |
| 碰撞检测 | `SweepMultiByChannel` | 自定义碰撞算法 |
| 动画播放 | `UAnimMontage` + Notify | 帧动画系统 |
| AI 感知 | `UAIPerceptionComponent` | 视野检测算法 |
| 音频播放 | `UAudioComponent` | 音频引擎 |
| UI 显示 | `UMG` + `UUserWidget` | 自定义渲染 |

---

## 3. 团队分工文档 (Roles & Responsibilities)

### 3.1 角色分配总览

```
┌──────────────────────────────────────────────────────────────┐
│                        团队结构                               │
├──────────────┬───────────────────────────────────────────────┤
│  Member A    │  组长 / 主角系统                               │
│  (组长)      │  AWukongCharacter, 输入, 连招, 闪避            │
├──────────────┼───────────────────────────────────────────────┤
│  Member B    │  AI / 敌人系统                                 │
│              │  AEnemyBase, 敌人子类, AI Controller           │
├──────────────┼───────────────────────────────────────────────┤
│  Member C    │  战斗 / 动画系统                               │
│              │  UCombatComponent, 伤害计算, Hitbox, 技能      │
├──────────────┼───────────────────────────────────────────────┤
│  Member D    │  系统 / 关卡 / UI                              │
│              │  UWukongHUD, 场景切换, 音乐, 菜单              │
└──────────────┴───────────────────────────────────────────────┘
```

### 3.2 Member A - 主角系统

**负责类**：
- `AWukongCharacter` - 主角控制
- `UWukongAnimInstance` - 动画实例
- `FWukongStateMachine` - 状态机（非 UObject）

**核心职责**：
1. 实现增强输入系统绑定（Enhanced Input）
2. 实现连招输入缓冲队列
3. 实现闪避（无敌帧 + 冷却）
4. 管理角色状态机转换
5. 与 `UCombatComponent` 协作触发攻击

**接口契约**：
```cpp
// 提供给其他成员调用的接口
void StartAttack();                    // Member C 调用
void ReceiveDamage(float Damage);      // Member C 调用
void SetInvincible(bool bInvincible);  // 内部使用
FOnHealthChanged OnHealthChanged;      // Member D 监听
```

### 3.3 Member B - AI/敌人系统

**负责类**：
- `AEnemyBase` - 敌人抽象基类
- `AEnemyMelee` - 近战敌人
- `AEnemyRanged` - 远程敌人
- `AEnemySkirmisher` - 机动敌人
- `ABossCharacter` - Boss
- `AEnemyAIController` - AI 控制器

**核心职责**：
1. 设计敌人继承体系（多态）
2. 实现 AI 状态机（追击/攻击/闪避）
3. 实现 Boss 阶段切换逻辑
4. 实现敌人感知系统

**接口契约**：
```cpp
// 抽象接口（子类必须实现）
virtual void ExecuteAttack() PURE_VIRTUAL(...);
virtual void TickAI(float DeltaTime);

// 提供给其他成员调用
void ReceiveDamage(float Damage, AActor* Instigator);  // Member C 调用
void SetTarget(AActor* NewTarget);                      // 内部使用
FOnEnemyDeath OnEnemyDeath;                            // Member D 监听
```

### 3.4 Member C - 战斗/动画系统

**负责类**：
- `UCombatComponent` - 战斗组件
- `UHitboxComponent` - 攻击判定组件
- `UWukongAnimNotify` - 动画通知
- 模板类 `TDamageCalculator<T>` - 伤害计算

**核心职责**：
1. 实现 Hitbox/Hurtbox 检测系统
2. 实现伤害计算（使用模板）
3. 实现硬直/击退效果
4. 实现技能系统（棒击震地）
5. 动画通知与 C++ 逻辑绑定

**接口契约**：
```cpp
// UCombatComponent 接口
void PerformAttackTrace(float Radius, float Range);
void ActivateSkill(FName SkillName);
bool IsSkillOnCooldown(FName SkillName) const;

// 委托
FOnHitDetected OnHitDetected;          // 命中回调
FOnSkillActivated OnSkillActivated;    // 技能激活
```

### 3.5 Member D - 系统/关卡/UI

**负责类**：
- `UWukongHUD` - 主 HUD Widget
- `UWukongGameInstance` - 游戏实例
- `ALevelTriggerVolume` - 场景切换触发器
- `UWukongAudioManager` - 音频管理

**核心职责**：
1. 实现所有 UI 更新逻辑（C++）
2. 实现场景切换系统
3. 实现音乐切换系统
4. 实现暂停/重开逻辑
5. 实现游戏状态管理

**接口契约**：
```cpp
// UWukongHUD 接口
void UpdatePlayerHealth(float Current, float Max);
void UpdateBossHealth(float Current, float Max);
void ShowBossHealthBar(bool bShow);
void UpdateSkillCooldown(FName SkillName, float Percent);

// UWukongGameInstance 接口
void PauseGame();
void ResumeGame();
void RestartLevel();
void LoadLevel(FName LevelName);
```

### 3.6 类依赖关系图

```
                    ┌─────────────────┐
                    │  UGameInstance  │
                    │   (Member D)    │
                    └────────┬────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ AWukongCharacter│ │   AEnemyBase    │ │   UWukongHUD    │
│   (Member A)    │ │   (Member B)    │ │   (Member D)    │
└────────┬────────┘ └────────┬────────┘ └─────────────────┘
         │                   │                   ▲
         │ has               │ has               │ updates
         ▼                   ▼                   │
┌─────────────────┐ ┌─────────────────┐          │
│UCombatComponent │ │AEnemyAIController│         │
│   (Member C)    │ │   (Member B)    │          │
└────────┬────────┘ └─────────────────┘          │
         │                                       │
         │ fires events ─────────────────────────┘
         │
         ▼
┌─────────────────┐
│ TDamageCalculator│
│   (Member C)    │
└─────────────────┘
```

---

## 4. 短期与长期目标 (Timeline)

### 4.1 整体时间线

```
Week 1 (12/02-12/08)     Week 2 (12/09-12/15)     Week 3 (12/16-12/22)     Week 4 (12/23-12/28)
─────────────────────    ─────────────────────    ─────────────────────    ─────────────────────
    【地基期】               【骨架期】               【血肉期】               【打磨期】
                        
• 环境配置完成            • 连招系统完成            • Boss 实装              • Bug 修复
• 核心类骨架              • 3种敌人 AI             • 阶段切换               • 性能优化
• 基础移动/攻击           • 伤害/硬直系统          • 完整 UI                • 最终整合
• 日志验证                • HUD 框架               • 场景切换               • 打包测试
```

### 4.2 Week 1 详细计划（地基期）

**目标**：搭建可编译运行的框架，验证核心流程

| 成员 | 必须完成的类 | 验收标准 |
|-----|-------------|---------|
| A | `AWukongCharacter` 基础 | 移动/跳跃正常，按闪避键打印日志 |
| B | `AEnemyBase` + `AEnemyMelee` | Spawn 后能追踪玩家，打印状态日志 |
| C | `UCombatComponent` 基础 | 按攻击键执行 Trace，打印命中日志 |
| D | `UWukongHUD` + `ALevelTriggerVolume` | 血条显示，进入 Trigger 打印日志 |

**公共任务**：
- [ ] 配置 Git LFS（`.uasset`, `.umap`）
- [ ] 建立分支规范（`feature/<name>`）
- [ ] 统一代码风格（UE 命名规范）

**Week 1 验收 Demo**：
> 在 Editor 中运行，角色可移动，按攻击键触发 Trace 并在 Output 打印结果；场景中有一个敌人会追踪玩家。

### 4.3 Week 2 详细计划（骨架期）

**目标**：核心战斗循环可玩

| 成员 | 任务 | 验收标准 |
|-----|------|---------|
| A | 完善连招（3段）+ 输入缓冲 | 连续按攻击可打出 3 段连招 |
| B | 实现 3 种敌人 + 完整 AI | 敌人有追击/攻击/闪避行为 |
| C | Hitbox + 伤害 + 硬直 + 技能 | 攻击能造成伤害，敌人有硬直 |
| D | 血条更新 + 技能 CD 显示 | UI 实时反映游戏状态 |

**Week 2 验收 Demo**：
> 角色可用连招和技能击杀 3 种不同敌人，UI 显示双方血量和技能冷却。

### 4.4 Week 3 详细计划（血肉期）

**目标**：Boss 战可玩，场景完整

| 成员 | 任务 | 验收标准 |
|-----|------|---------|
| A | Boss 战配合调整 | 角色能应对 Boss 攻击 |
| B | `ABossCharacter` + 阶段切换 | Boss 血量 <50% 进入第二阶段 |
| C | Boss 技能判定 | Boss 攻击有正确的 Hitbox |
| D | Boss 血条 + 场景切换 + 音乐 | 进入 Boss 区域切换音乐和 UI |

**Week 3 验收 Demo**：
> 从起始区域走到 Boss 区域，触发 Boss 战，击败多阶段 Boss。

### 4.5 Week 4 详细计划（打磨期）

**目标**：稳定可演示的最终版本

| 任务 | 负责人 | 说明 |
|-----|--------|-----|
| Bug 修复 | 全员 | 修复已知问题 |
| 代码审查 | 全员 | 确保无蓝图逻辑 |
| 性能检查 | A+C | 检查 Tick 开销 |
| UI 打磨 | D | 菜单流程完整 |
| 打包测试 | D | Windows 可执行包 |
| 演示准备 | A | 录制演示视频 |

**最终交付物**：
- [ ] Windows 可执行包（Shipping Build）
- [ ] 源码仓库（编译通过）
- [ ] README（运行说明）
- [ ] 5-10 分钟演示视频

---

## 5. 技术避坑指南 (Survival Guide)

### 5.1 UE 容器 vs STL 对照表

> ⚠️ **UE 中禁止直接使用 STL 容器管理 UObject**

| 你想做的事 | ❌ 不要用 | ✅ 应该用 |
|-----------|----------|---------|
| 动态数组 | `std::vector` | `TArray<T>` |
| 键值对 | `std::map` | `TMap<K,V>` |
| 集合 | `std::set` | `TSet<T>` |
| 队列 | `std::queue` | `TQueue<T>` 或 `TArray` + 索引 |
| 字符串 | `std::string` | `FString`, `FName`, `FText` |
| 独占指针 | `std::unique_ptr` | `TUniquePtr<T>` |
| 共享指针 | `std::shared_ptr` | `TSharedPtr<T>` |
| 弱指针 | `std::weak_ptr` | `TWeakPtr<T>` |
| 可调用对象 | `std::function` | `TFunction<Sig>` |
| 移动 | `std::move` | `MoveTemp()` |

### 5.2 UE5 指针使用规范

> ⚠️ **UE5 推荐使用 `TObjectPtr<T>` 替代裸指针**

```cpp
// ❌ UE4 风格（仍可用但不推荐）
UPROPERTY()
AEnemyBase* Enemy;

// ✅ UE5 推荐风格
UPROPERTY()
TObjectPtr<AEnemyBase> Enemy;

// ✅ 跨帧弱引用（目标可能被销毁）
TWeakObjectPtr<AActor> Target;

// ✅ 软引用（延迟加载资产）
UPROPERTY()
TSoftObjectPtr<UTexture2D> IconTexture;

// ✅ 类引用（用于 Spawn）
UPROPERTY(EditDefaultsOnly)
TSubclassOf<AEnemyBase> EnemyClass;
```

**指针选择决策树**：

```
需要引用 UObject?
    │
    ├─ 是 ──→ 需要阻止 GC 回收?
    │            │
    │            ├─ 是 ──→ TObjectPtr<T> + UPROPERTY()
    │            │
    │            └─ 否 ──→ TWeakObjectPtr<T>
    │
    └─ 否 ──→ 需要共享所有权?
                 │
                 ├─ 是 ──→ TSharedPtr<T>
                 │
                 └─ 否 ──→ TUniquePtr<T>
```

### 5.3 常见崩溃及解决方案

#### 崩溃 1：野指针（最常见）

```cpp
// ❌ 错误：未标记 UPROPERTY，GC 会回收
AEnemyBase* Enemy;  // 某帧后变成野指针！

// ✅ 正确：UPROPERTY 让 GC 追踪
UPROPERTY()
TObjectPtr<AEnemyBase> Enemy;
```

#### 崩溃 2：访问已销毁对象

```cpp
// ❌ 错误：直接访问可能已销毁的 Actor
Target->GetActorLocation();

// ✅ 正确：先检查有效性
if (IsValid(Target))
{
    Target->GetActorLocation();
}

// ✅ 或使用弱引用
TWeakObjectPtr<AActor> WeakTarget = Target;
if (WeakTarget.IsValid())
{
    WeakTarget->GetActorLocation();
}
```

#### 崩溃 3：构造函数中访问 World

```cpp
// ❌ 错误：构造函数中 GetWorld() 返回 nullptr
AWukongCharacter::AWukongCharacter()
{
    GetWorld()->SpawnActor(...);  // 崩溃！
}

// ✅ 正确：在 BeginPlay 中访问
void AWukongCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->SpawnActor(...);  // OK
}
```

#### 崩溃 4：非主线程访问 UObject

```cpp
// ❌ 错误：异步任务中直接访问 UObject
AsyncTask(ENamedThreads::AnyThread, [this]()
{
    this->Health -= 10;  // 线程不安全！
});

// ✅ 正确：回到主线程执行
AsyncTask(ENamedThreads::AnyThread, [this]()
{
    // 计算完成后回到主线程
    AsyncTask(ENamedThreads::GameThread, [this]()
    {
        this->Health -= 10;  // OK
    });
});
```

### 5.4 版本控制规范

#### Git LFS 配置

在项目根目录创建 `.gitattributes`：

```gitattributes
# Unreal Engine
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text
*.uexp filter=lfs diff=lfs merge=lfs -text
*.ubulk filter=lfs diff=lfs merge=lfs -text

# 二进制资源
*.png filter=lfs diff=lfs merge=lfs -text
*.jpg filter=lfs diff=lfs merge=lfs -text
*.wav filter=lfs diff=lfs merge=lfs -text
*.mp3 filter=lfs diff=lfs merge=lfs -text
*.fbx filter=lfs diff=lfs merge=lfs -text
```

#### .gitignore 必须包含

```gitignore
# UE 生成目录
Binaries/
Intermediate/
Saved/
DerivedDataCache/

# VS 生成
.vs/
*.sln
*.suo

# 本地配置
*.user
```

#### 分支策略

```
main          ←── 只合并稳定版本
  │
  └── dev     ←── 开发主分支，每周合并一次到 main
       │
       ├── feature/player-combo      (Member A)
       ├── feature/enemy-ai          (Member B)
       ├── feature/combat-system     (Member C)
       └── feature/ui-system         (Member D)
```

### 5.5 动画资源与 C++ 整合

#### 动画通知绑定流程

```
1. 美术在 AnimMontage 中添加 AnimNotify
   名称约定：AN_<功能>，如 AN_AttackHit

2. C++ 创建对应的 UAnimNotify 子类
   class UAN_AttackHit : public UAnimNotify

3. 在 Notify 的 Notify() 函数中调用战斗逻辑
   OwnerComp->PerformAttackTrace();

4. AnimMontage 中选择 C++ 创建的 Notify 类
```

#### 代码示例：动画通知

```cpp
// AN_AttackHit.h
UCLASS()
class UAN_AttackHit : public UAnimNotify
{
    GENERATED_BODY()
    
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, 
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
    
    UPROPERTY(EditAnywhere)
    float DamageRadius = 100.f;
};

// AN_AttackHit.cpp
void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, 
                           UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            Combat->PerformAttackTrace(DamageRadius, 150.f);
        }
    }
}
```

### 5.6 性能注意事项

| 操作 | 开销 | 建议 |
|-----|------|-----|
| `Tick` 中 LineTrace | 高 | 仅在攻击帧执行，用 AnimNotify 触发 |
| `TArray::Add` 频繁调用 | 中 | 预分配 `Reserve()` |
| `FindComponentByClass` | 中 | 缓存结果，不要每帧调用 |
| 大量 Actor Spawn/Destroy | 高 | 使用对象池 |
| GC 频繁触发 | 高 | 减少临时 UObject 创建 |

---

## 6. 附录：核心类骨架代码

### 6.1 AWukongCharacter（Member A）

```cpp
// WukongCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WukongCharacter.generated.h"

// 前置声明
class UCombatComponent;
class UInputAction;
class UInputMappingContext;

// 角色状态枚举
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

// 血量变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

UCLASS()
class YOURPROJECT_API AWukongCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AWukongCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // ========== 公开接口 ==========
    
    /** 接收伤害（Member C 调用） */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReceiveDamage(float Damage, AActor* Instigator);
    
    /** 设置无敌状态 */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvincible(bool bNewInvincible);
    
    /** 获取当前状态 */
    UFUNCTION(BlueprintPure, Category = "State")
    EWukongState GetCurrentState() const { return CurrentState; }

    // ========== 委托 ==========
    
    /** 血量变化事件（Member D 监听） */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChanged OnHealthChanged;

protected:
    virtual void BeginPlay() override;

    // ========== 组件 ==========
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCombatComponent> CombatComponent;

    // ========== 属性 ==========
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float MaxHealth = 100.f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DodgeCooldown = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DodgeDistance = 400.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DodgeInvincibleDuration = 0.3f;

    // ========== 输入（Enhanced Input） ==========
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> AttackAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> DodgeAction;

private:
    // ========== 状态管理 ==========
    
    EWukongState CurrentState = EWukongState::Idle;
    
    void SetState(EWukongState NewState);
    
    // ========== 连招系统 ==========
    
    /** 输入缓冲队列（使用 TArray 模拟队列） */
    TArray<FName> InputBuffer;
    
    int32 CurrentComboIndex = 0;
    
    bool bCanAcceptComboInput = false;
    
    void ProcessInputBuffer();
    
    void ResetCombo();

    // ========== 闪避系统 ==========
    
    bool bCanDodge = true;
    
    bool bIsInvincible = false;
    
    FTimerHandle DodgeCooldownTimer;
    
    FTimerHandle InvincibleTimer;
    
    void ExecuteDodge();
    
    void OnDodgeCooldownEnd();
    
    void OnInvincibleEnd();

    // ========== 输入回调 ==========
    
    void HandleMoveInput(const struct FInputActionValue& Value);
    
    void HandleJumpInput();
    
    void HandleAttackInput();
    
    void HandleDodgeInput();
};
```

### 6.2 AEnemyBase（Member B）

```cpp
// EnemyBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

// AI 状态枚举
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    Idle,
    Patrol,
    Chase,
    Attack,
    Stagger,
    Dead
};

// 敌人死亡委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, AEnemyBase*, Enemy);

/**
 * 敌人抽象基类 - 所有敌人继承此类
 * 展示多态：子类必须实现 ExecuteAttack()
 */
UCLASS(Abstract)
class YOURPROJECT_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

    virtual void Tick(float DeltaTime) override;

    // ========== 纯虚函数（多态） ==========
    
    /** 执行攻击 - 子类必须实现 */
    virtual void ExecuteAttack() PURE_VIRTUAL(AEnemyBase::ExecuteAttack, );

    // ========== 虚函数（可重写） ==========
    
    /** AI 行为更新 */
    virtual void TickAI(float DeltaTime);
    
    /** 接收伤害 */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void ReceiveDamage(float Damage, AActor* Instigator, const FHitResult& HitInfo);

    // ========== 公开接口 ==========
    
    UFUNCTION(BlueprintPure, Category = "State")
    bool IsDead() const { return CurrentHealth <= 0.f; }
    
    UFUNCTION(BlueprintPure, Category = "State")
    EEnemyAIState GetAIState() const { return AIState; }
    
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    // ========== 委托 ==========
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyDeath OnEnemyDeath;

protected:
    virtual void BeginPlay() override;

    // ========== 属性 ==========
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float MaxHealth = 100.f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentHealth;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float DetectionRange = 1000.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float AttackRange = 150.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float ChaseSpeed = 400.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float StaggerDuration = 0.5f;

    // ========== AI 状态 ==========
    
    EEnemyAIState AIState = EEnemyAIState::Idle;
    
    /** 使用弱引用，目标销毁时自动失效 */
    TWeakObjectPtr<AActor> CurrentTarget;
    
    void SetAIState(EEnemyAIState NewState);

    // ========== AI 行为（子类可重写） ==========
    
    virtual void TickIdle(float DeltaTime);
    virtual void TickChase(float DeltaTime);
    virtual void TickAttack(float DeltaTime);
    virtual void OnStaggerEnd();

private:
    FTimerHandle StaggerTimer;
    
    void Die();
};

// ==================== 具体敌人类型 ====================

/**
 * 近战敌人
 */
UCLASS()
class YOURPROJECT_API AEnemyMelee : public AEnemyBase
{
    GENERATED_BODY()

public:
    virtual void ExecuteAttack() override;
    virtual void TickAI(float DeltaTime) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeDamage = 15.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float AttackCooldown = 2.0f;

private:
    float LastAttackTime = 0.f;
};

/**
 * 远程敌人
 */
UCLASS()
class YOURPROJECT_API AEnemyRanged : public AEnemyBase
{
    GENERATED_BODY()

public:
    virtual void ExecuteAttack() override;
    virtual void TickAI(float DeltaTime) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float ProjectileSpeed = 1000.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float PreferredDistance = 600.f;
};

/**
 * 机动敌人（会闪避）
 */
UCLASS()
class YOURPROJECT_API AEnemySkirmisher : public AEnemyBase
{
    GENERATED_BODY()

public:
    virtual void ExecuteAttack() override;
    virtual void TickAI(float DeltaTime) override;
    virtual void ReceiveDamage(float Damage, AActor* Instigator, const FHitResult& HitInfo) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DodgeChance = 0.3f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float DodgeDistance = 300.f;

private:
    void PerformDodge(const FVector& ThreatDirection);
};
```

### 6.3 UCombatComponent（Member C）

```cpp
// CombatComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// 命中检测委托
DECLARE_DELEGATE_TwoParams(FOnHitDetected, AActor* /*HitActor*/, const FHitResult& /*HitResult*/);

// ==================== 模板类：伤害计算器 ====================

/** 伤害策略基础 */
struct FDamagePolicy
{
    virtual ~FDamagePolicy() = default;
    virtual float Apply(float BaseDamage) const = 0;
};

/** 物理伤害策略 */
struct FPhysicalDamagePolicy : public FDamagePolicy
{
    float ArmorReduction = 0.1f;
    
    virtual float Apply(float BaseDamage) const override
    {
        return BaseDamage * (1.f - ArmorReduction);
    }
};

/** 魔法伤害策略 */
struct FMagicDamagePolicy : public FDamagePolicy
{
    float Multiplier = 1.5f;
    
    virtual float Apply(float BaseDamage) const override
    {
        return BaseDamage * Multiplier;
    }
};

/** 模板伤害计算器 - 展示 C++ 模板使用 */
template<typename TPolicy>
class TDamageCalculator
{
public:
    static float Calculate(float BaseDamage, const TPolicy& Policy)
    {
        return Policy.Apply(BaseDamage);
    }
};

// ==================== 战斗组件 ====================

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class YOURPROJECT_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

    // ========== 攻击检测 ==========
    
    /** 执行攻击范围检测 */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttackTrace(float Radius, float Range);
    
    /** 清除本次攻击已命中的目标 */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearHitActors();

    // ========== 技能系统 ==========
    
    /** 激活技能 */
    UFUNCTION(BlueprintCallable, Category = "Skill")
    bool ActivateSkill(FName SkillName);
    
    /** 检查技能是否在冷却 */
    UFUNCTION(BlueprintPure, Category = "Skill")
    bool IsSkillOnCooldown(FName SkillName) const;
    
    /** 获取技能剩余冷却时间 */
    UFUNCTION(BlueprintPure, Category = "Skill")
    float GetSkillRemainingCooldown(FName SkillName) const;

    // ========== 伤害计算（模板方法） ==========
    
    template<typename TPolicy>
    float CalculateDamage(float BaseDamage, const TPolicy& Policy)
    {
        return TDamageCalculator<TPolicy>::Calculate(BaseDamage, Policy);
    }

    // ========== 委托 ==========
    
    FOnHitDetected OnHitDetected;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ========== 配置 ==========
    
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float BaseDamage = 20.f;
    
    /** 技能冷却时间配置表 */
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    TMap<FName, float> SkillCooldownConfig;

private:
    /** 本次攻击已命中的目标（避免重复伤害） */
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActorsThisSwing;
    
    /** 技能当前冷却剩余时间 */
    TMap<FName, float> SkillCooldownRemaining;
    
    /** 缓存的命中结果 */
    TArray<FHitResult> CachedHits;
    
    void ProcessHits(const TArray<FHitResult>& Hits);
    
    void ApplyDamageToActor(AActor* Target, float Damage, const FHitResult& HitInfo);
    
    void UpdateSkillCooldowns(float DeltaTime);
    
    // ========== 具体技能实现 ==========
    
    void ExecuteSkill_GroundSlam();
};
```

### 6.4 UWukongHUD（Member D）

```cpp
// WukongHUD.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WukongHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

/**
 * 主 HUD Widget - 所有 UI 更新逻辑在 C++ 中实现
 */
UCLASS()
class YOURPROJECT_API UWukongHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // ========== 玩家血条 ==========
    
    UFUNCTION(BlueprintCallable, Category = "UI|Health")
    void UpdatePlayerHealth(float Current, float Max);

    // ========== Boss 血条 ==========
    
    UFUNCTION(BlueprintCallable, Category = "UI|Boss")
    void ShowBossHealthBar(bool bShow);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Boss")
    void UpdateBossHealth(float Current, float Max);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Boss")
    void SetBossName(const FText& Name);

    // ========== 技能冷却 ==========
    
    UFUNCTION(BlueprintCallable, Category = "UI|Skill")
    void UpdateSkillCooldown(FName SkillName, float RemainingTime, float TotalCooldown);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Skill")
    void RegisterSkillSlot(FName SkillName, UProgressBar* CooldownBar, UImage* SkillIcon);

    // ========== 系统消息 ==========
    
    UFUNCTION(BlueprintCallable, Category = "UI|Message")
    void ShowDeathScreen();
    
    UFUNCTION(BlueprintCallable, Category = "UI|Message")
    void HideDeathScreen();

protected:
    virtual void NativeConstruct() override;

    // ========== 绑定的 Widget（在 UMG 中设置） ==========
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> PlayerHealthBar;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> BossHealthBar;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> BossNameText;
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> BossHealthContainer;
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidget> DeathScreenContainer;

private:
    /** 技能冷却 UI 映射表 */
    TMap<FName, TObjectPtr<UProgressBar>> SkillCooldownBars;
    
    TMap<FName, TObjectPtr<UImage>> SkillIcons;
    
    /** 插值平滑血条变化 */
    float TargetPlayerHealthPercent = 1.f;
    float TargetBossHealthPercent = 1.f;
    
    void InterpHealthBars(float DeltaTime);
};
```

### 6.5 项目技术亮点总结（向老师展示）

```cpp
// ============================================================
// 项目 C++ 技术点总结 - 便于老师评分参考
// ============================================================

/*
 * 1. 【容器】使用 UE 容器（等价于 STL）
 *    - TArray<T>     → 替代 std::vector
 *    - TMap<K,V>     → 替代 std::map/unordered_map
 *    - TSet<T>       → 替代 std::set
 *    - TQueue<T>     → 替代 std::queue
 *    
 *    示例位置：
 *    - AWukongCharacter::InputBuffer (TArray)
 *    - UCombatComponent::SkillCooldownConfig (TMap)
 *    - UCombatComponent::HitActorsThisSwing (TSet)
 */

/*
 * 2. 【多态】抽象基类 + 虚函数重写
 *    
 *    示例位置：
 *    - AEnemyBase（抽象基类）
 *      ├── AEnemyMelee（近战实现）
 *      ├── AEnemyRanged（远程实现）
 *      └── AEnemySkirmisher（机动实现）
 *    
 *    纯虚函数：virtual void ExecuteAttack() PURE_VIRTUAL(...);
 *    虚函数重写：virtual void TickAI(float DeltaTime) override;
 */

/*
 * 3. 【模板】自定义模板类
 *    
 *    示例位置：
 *    - TDamageCalculator<TPolicy>（伤害计算模板）
 *    - UCombatComponent::CalculateDamage<TPolicy>()（模板成员函数）
 */

/*
 * 4. 【C++11/14/17 特性】
 *    
 *    - auto 类型推导
 *    - Lambda 表达式（TArray::RemoveAll）
 *    - 基于范围的 for 循环
 *    - MoveTemp()（等价于 std::move）
 *    - TFunction<Sig>（等价于 std::function）
 *    - TUniquePtr / TSharedPtr（智能指针）
 *    - 初始化列表
 *    - override / final 关键字
 */

/*
 * 5. 【委托/事件系统】
 *    
 *    示例位置：
 *    - FOnHealthChanged（动态多播委托）
 *    - FOnEnemyDeath（动态多播委托）
 *    - FOnHitDetected（普通委托）
 */
```

---

## 附录：快速参考卡片

### A. 指针选择速查

| 场景 | 使用 |
|-----|------|
| 组件指针（UPROPERTY） | `TObjectPtr<UComponent>` |
| Actor 引用（UPROPERTY） | `TObjectPtr<AActor>` |
| 可能被销毁的目标 | `TWeakObjectPtr<AActor>` |
| 资产软引用 | `TSoftObjectPtr<UObject>` |
| 类引用（Spawn用） | `TSubclassOf<AActor>` |
| 非 UObject 独占 | `TUniquePtr<T>` |
| 非 UObject 共享 | `TSharedPtr<T>` |

### B. 常用宏

```cpp
UPROPERTY(EditDefaultsOnly)     // 仅在蓝图默认值中可编辑
UPROPERTY(EditAnywhere)         // 任何地方可编辑
UPROPERTY(BlueprintReadOnly)    // 蓝图只读
UPROPERTY(BlueprintReadWrite)   // 蓝图可读写
UPROPERTY(VisibleAnywhere)      // 可见但不可编辑

UFUNCTION(BlueprintCallable)    // 蓝图可调用
UFUNCTION(BlueprintPure)        // 蓝图纯函数（无副作用）
UFUNCTION(BlueprintImplementableEvent)  // 蓝图实现
UFUNCTION(BlueprintNativeEvent)         // C++ 默认实现，蓝图可重写

UCLASS(Abstract)                // 抽象类
UCLASS(Blueprintable)           // 可被蓝图继承
```

### C. 日志打印

```cpp
// 普通日志
UE_LOG(LogTemp, Log, TEXT("Message: %s"), *SomeString);

// 警告
UE_LOG(LogTemp, Warning, TEXT("Warning: %d"), SomeInt);

// 错误
UE_LOG(LogTemp, Error, TEXT("Error: %f"), SomeFloat);

// 屏幕打印（调试用）
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Debug!"));
```

---

> 📅 文档最后更新：2024/12/02  
> 📝 版本：1.0  
> 👥 适用团队：4人开发小组
