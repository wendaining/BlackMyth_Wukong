# 死亡重生系统实现说明

## 功能概述
实现了一个完整的角色死亡和重生系统，包括：
1. 角色死亡时显示死亡菜单
2. 菜单包含"重生"和"退出游戏"两个选项
3. 与Temple交互时自动保存为重生点
4. 点击重生后在保存的重生点复活

## 修改的文件

### 1. BlackMythGameInstance.h 和 .cpp
**功能**: 在游戏实例中存储重生点数据

**新增内容**:
- `bHasRespawnPoint`: 是否有保存的重生点
- `RespawnLocation`: 重生点位置
- `RespawnRotation`: 重生点旋转
- `RespawnTempleID`: 重生点所属的Temple ID
- `SetRespawnPoint()`: 设置重生点的函数
- `HasRespawnPoint()`: 检查是否有重生点的函数

### 2. DeathMenuWidget.h 和 .cpp (新建)
**功能**: 死亡菜单UI类

**主要功能**:
- `OnRespawnClicked()`: 处理重生按钮点击
- `OnQuitGameClicked()`: 处理退出游戏按钮点击
- 自动绑定UI按钮和文本控件

**UI组件**:
- `RespawnButton`: 重生按钮
- `QuitGameButton`: 退出游戏按钮
- `RespawnButtonText`: 重生按钮文本（可选）
- `QuitGameButtonText`: 退出游戏按钮文本（可选）

### 3. Temple.h 和 .cpp
**功能**: 在与土地庙交互时保存重生点

**新增内容**:
- `SaveRespawnPoint()`: 保存重生点到GameInstance的函数
- 在 `OnInteract_Implementation()` 中调用保存重生点

**工作流程**:
1. 玩家与Temple交互
2. 恢复生命和体力
3. 获取TeleportPoint组件的位置和旋转
4. 保存到GameInstance作为重生点

### 4. WukongCharacter.h 和 .cpp
**功能**: 实现角色死亡处理和重生逻辑

**新增内容**:
- `DeathMenuWidgetClass`: 死亡菜单Widget类（需在蓝图中配置）
- `DeathMenuInstance`: 当前显示的死亡菜单实例
- `ShowDeathMenu()`: 显示死亡菜单的函数
- `Respawn()`: 重生函数

**死亡流程**:
1. 角色死亡时调用 `Die()`
2. 延迟2秒后调用 `ShowDeathMenu()`
3. 显示死亡菜单，暂停游戏
4. 切换到UI输入模式，显示鼠标光标

**重生流程**:
1. 从GameInstance获取重生点数据
2. 重置角色状态为Idle
3. 恢复动画蓝图模式
4. 恢复碰撞和玩家输入
5. 完全恢复生命值和体力
6. 传送到重生点位置

### 5. BlackMythSaveGame.h
**功能**: 添加重生点数据到存档系统（用于持久化存储）

**新增字段**:
- `bHasRespawnPoint`: 是否有保存的重生点
- `RespawnLocation`: 重生点位置
- `RespawnRotation`: 重生点旋转
- `RespawnTempleID`: 重生点所属的Temple ID

### 测试流程

1. **设置重生点**：
   - 靠近一个Temple
   - 按G键与Temple交互
   - 系统会自动保存当前Temple的TeleportPoint位置为重生点

2. **触发死亡**：
   - 让角色生命值降为0
   - 死亡动画播放
   - 2秒后显示死亡菜单

3. **重生**：
   - 点击"重生"按钮
   - 角色在保存的Temple位置复活
   - 生命值和体力值完全恢复

4. **退出游戏**：
   - 点击"退出游戏"按钮
   - 游戏关闭
