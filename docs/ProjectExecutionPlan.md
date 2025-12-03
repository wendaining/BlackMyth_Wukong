> 注：只是AI打的一个草稿，还没仔细规整过

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

待完成

## 4. 短期与长期目标 (Timeline)

待完成

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

待实现

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

> 📅 文档最后更新：2024/12/03
> 📝 版本：1.0  
> 👥 适用团队：4人开发小组
