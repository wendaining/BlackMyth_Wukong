# BlackMyth_Wukong——仿知名游戏黑神话悟空的课程大作业

> [!CAUTION]
>
> 1. **不要直接git clone！！！直接Clone下来的文件夹大小高达80GB左右，直接git clone会消耗大量Git LFS带宽，因此有需要的部分可以自行选取下载，也可以到[release](https://github.com/wendaining/BlackMyth_Wukong/releases/tag/v0.1.0)去下载可以直接游玩的版本。**
> 2. 本游戏的美术资源均获取于网络，请勿商用

## 项目概述

本项目参考同名游戏《黑神话·悟空》，**基于Unreal Engine 5.4**实现了一款第三人称动作冒险游戏。游戏以东方神话为主题，玩家操纵“悟空”角色，在3D场景中进行探索与战斗。项目实现了核心的战斗系统（攻击、闪避、法术）、敌人AI（普通怪物与Boss二郎神）、以及完整的游戏循环（存档、商店、场景切换）。

## Release
[v0.1.0](https://github.com/wendaining/BlackMyth_Wukong/releases/tag/v0.1.0)
目前仅在Windows11上测试运行过，注意采用了分卷压缩的形式。

## 项目组成
项目主要目录结构如下：

- **/docs**
  - 包含所有项目文档。
- **/BlackMyth/Source/BlackMyth**
  - **核心代码库**，包含所有 C++ 类实现：
    - `/Components`: 通用组件（生命、体力、背包、状态等）。
    - `/Combat`: 战斗相关逻辑。
    - `/AI`: 敌人行为树与控制器。
    - `/UI`: 所有 Widget 的 C++ 后端逻辑。
    - `/Items`: 道具类定义。
- **/BlackMyth/Content**
  - **美术资产**：包含模型、材质、动画、音效、蓝图等。

## 项目文档

详见 [项目说明文档](docs/ProjectDoc.md)

**[项目加分项文档](docs/BonusPoints.md)**

## 操作说明

| **按键**          | **功能描述**                         |
| ----------------- | ------------------------------------ |
| **W / A / S / D** | 角色移动                             |
| **Shift (按住)**  | 疾跑                                 |
| **鼠标左键**      | 轻击                                 |
| **鼠标右键**      | 重击                                 |
| **鼠标中键**      | 锁定敌人                             |
| **Ctrl**          | 翻滚 (躲避)                          |
| **空格**  | 跳跃                                 |
| **X**             | 立棍击打                             |
| **V**             | 甩花棍 (防御/连段)                   |
| **F**             | 拾取金币 / 交互                      |
| **I**             | 打开背包                             |
| **1 / 2 / 3 / 4** | 释放技能 / 使用背包道具 (打开背包后) |

## 部分演示

> [!IMPORTANT]
>
> 以下演示包含 gif 动图，请耐心等待加载完毕。

### 开始菜单

<img width="559" height="321" alt="image" src="https://github.com/user-attachments/assets/8cb54a60-8a29-4683-95d7-908546439da7" />

### 基础动作：走路、跑步

![基础动作：走路、跑步](https://github.com/user-attachments/assets/b90ba58e-d819-47c3-96de-ffc473c0cfb3)

### 基础战斗

![基础战斗](https://github.com/user-attachments/assets/1769ebb5-230c-4028-b153-9a9024bfa10d)

### 分身术

![分身术](https://github.com/user-attachments/assets/b37826b2-1bcd-4b24-a986-daab0dcb1948)

### 定身术

![定身术](https://github.com/user-attachments/assets/5df49a79-6b08-40b0-902f-ee827473c7b8)

### 变身术

![变身术](https://github.com/user-attachments/assets/1a09a077-aaf0-4780-84bc-ef525892a2cc)

### 安息术

![安息术](https://github.com/user-attachments/assets/0823e7fe-39e8-48de-a696-c04b17ec1d73)

### 使用道具

![使用道具](https://github.com/user-attachments/assets/a466a533-55c0-490e-9c5b-854f310ecadc)

### NPC对话

![NPC对话](https://github.com/user-attachments/assets/bc4fe622-ca7d-41f8-b5b8-31a499f475b5)

### Boss战前CG

![Boss战CG](https://github.com/user-attachments/assets/08a37ff5-f8a5-4fd0-a6e8-f52b0b177506)

### Boss战片段

![Boss战片段](https://github.com/user-attachments/assets/249ced8d-cf1b-4642-b97d-e6fefd812ef8)

### Boss二阶段

![Boss二阶段](https://github.com/user-attachments/assets/49c40a00-f231-4ad6-acf7-d958df5e2d95)

### 土地庙——传送系统

![土地庙——传送系统](https://github.com/user-attachments/assets/bd3410c9-f16e-4b2c-9a8d-54b941550cb0)

### 土地庙——商城系统

![土地庙——商城系统](https://github.com/user-attachments/assets/4254424a-2fe4-4769-8cea-945d31d3ec7a)

## 项目开发日志

### 2025-11-24

* [Git 协作规范](/docs/cooperatingSpecification.md)制定
* 明确分支管理策略
* 开始学习UE5.4的基础使用

### 2025-12-02

* UE5 项目工程建立
* 导入基础资产

### 2025-12-03

* 实现悟空基础移动（跑、跳）
* 实现基础攻击动作
* 引入 Paragon 资源包

### 2025-12-04

* 抽象出 `HealthComponent` 组件
* 抽象出 `StaminaComponent` 组件
* 抽象出 `CombatComponent` 组件
* 重构角色属性逻辑
* 实现生命值与体力值 UI

### 2025-12-05

* 引入 `EnemyBase` 基类
* 实现敌人基础感知 AI
* 实现敌人追踪 AI
* 增加项目设计架构文档

### 2025-12-08

* 完善攻击判定（TraceHitbox）
* 修复连招 Bug
* 修复位移 Bug
* 重构角色蓝图

### 2025-12-10

* 导入敌人模型与动画
* 实现混合空间（BlendSpace）
* 优化敌人追逐速度

### 2025-12-11

* 初步实现主菜单并对后续功能完成规划

### 2025-12-12

* 初步实现暂停菜单
* 初步实现存档/读档 UI 框架
* 增加敌人受击动画与音效
* 增加新敌人类型（斧头兵、剑兵）

### 2025-12-14

* 实现视角锁定（Lock-on）功能
* 优化战斗手感
* 增加被锁定敌人的视觉提示
* 修复敌人视野 Bug

### 2025-12-15

* 实现定身术技能
* 初步实现 NPC 对话系统
* 实现对话 UI 组件
* 增加新地图

### 2025-12-17

* 存档系统功能实装
* 实现状态效果（中毒/减速）
* 变身术（蝴蝶）实装
* 优化 NPC 交互逻辑
* 增加背景音乐

### 2025-12-20

* 二郎神 Boss 实装
* 实现 Boss 技能（召唤哮天犬）
* 实现 Boss 多阶段逻辑
* 背包系统上线
* 道具系统上线
* 土地庙交互功能实装

### 2025-12-21

* 土地庙传送功能实装
* 安息术、立棍AOE重构
* Boss 战空气墙实装
* 优化 Boss 战斗体验

### 2025-12-24

* 土地庙商店系统完成
* 金币掉落与拾取功能完成
* 死亡重生机制完善

### 2025-12-27

* 修复 Boss 硬直 Bug
* 修正地图问题
* 移除 Debug 线条
* 项目收尾与最终合并

### 2025-12-28

* 完成项目文档

## 文档更新日期

2025年12月28日
