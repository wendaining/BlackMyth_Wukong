**如果不熟悉以下说的一些git操作可以去这个网站玩一玩[Learn Git Branching](https://learngitbranching.js.org/?locale=zh_CN&NODEMO=)，我觉得开沙盒模式然后模拟一下我们的流程就行**

# git协作规范

**在做任何事情之前，首先在你的命令行里面输入`git lfs install`，再继续下面的内容。**

## 1. 仓库结构

保留**2+n**套主要分支：**main, dev, feature/, debug/\*, ...**

分工大致是：

- **main**：只在开发周期的关键节点更新
- **dev**：我们从这里拉分支开发（**但是我们禁止使用git pull，统一fetch+rebase**），完成后merge到这里
- **feature/\***：每个人/每个需求一个分支（这个暂定），如：
- **debug/\***: 一个bug开一个分支，
- ...，也许feature也可以细分，这个之后再说

```bash
feature/player-movement
feature/enemy-ai
...
```

- **千万不要直接动main，也不允许直接在dev上进行commit。**
- **基本上所有情况都是一个人负责同一个文件，防止merge conflict**，当然肯定会有例外。

## 2. 开发流程

### step1 

每次开发新的功能之前，必须确保你的起点是最新的（也就是先`fetch`一下），然后开新的分支

```bash
# 1. 更新远程信息
git fetch origin

# 2. 基于远程 dev 直接创建新分支
git checkout -b feature/你的功能名 origin/dev
```

### step 2

当你在自己的feature分支上工作时

直接正常，最简单的方式用git即可

```bash
git add xxx
git commit -m ""
```

> [!WARNING]
>
> **不要使用`add .`，不然容易产生很多冲突，我建议使用IDE自带的git来进行图形化的add操作**

**commit message有规范，第二部分会有讲**

> [!CAUTION]
>
> 强调一下，一定要多commit，哪怕只改了一点也commit，这个最后会作为工作依据的
>
> 另外，**关于推送备份：** 如果你想把没写完的代码推送到远程备份，可以使用 `git push -u origin 当前分支名。` **但是！** 一旦你推送到远程了，后续执行Step 3的Rebase操作后，再次推送时必须使用 **强制推送 `git push -f origin 你的分支名`**。

### step 3

打算将feature合并到dev上的时候

**必须让feature基于当前最新的dev以防止conflict**

```bash
git fetch origin
git rebase origin/dev
```

**`git rebase origin/dev`只允许在你这条工作分支完全结束，准备push的时候才允许执行！**

> [!TIP]
>
> **关于如何解决merge conflict**
>
> 1. IDE内解决冲突文件（变红的文件）。
>2. `git add 解决好的文件`
> 3. `git rebase --continue`
> 4. **千万不要** 在这里执行 `git commit`，也不要 `git merge`
> 5. 重复直到提示 Success

**变基完成后：** 如果你之前 push 过这个分支，现在需要强制更新远程分支：

```bash
git push -f origin 你的分支名
```

可以放心，这个`-f`在这条分支上是安全的。

### step 4

然后，将自己的工作合并到dev上

```bash
git checkout dev
git merge --no-ff 你自己的工作分支 -m "[Merge] message" //这里也有规范，下文和commit message的规范一块说
git push origin dev
```

**注意merge的--no-ff，这是为了清晰标明merge，防止fast-forward**

> [!TIP]
>
> 如果你在`git push -u origin 你的分支名`之后，github的页面出现了**Compare & PullRequest**，**可以无视**，这只是github的智能提醒，merge操作我们不需要通过github来进行
>
> 
> 当然，如果你不敢直接merge，希望由队友来进行Code Review，这个时候你可以让队友们来github页面上来review。

> [!TIP]
>
> 如果你确信这条分支在合并入dev之后不会再有任何用处，可以执行
>
> ```bash
> git branch -d 你的分支名
> git push origin --delete 你的分支名
> ```
>
> 以保持提交树的清爽。
>
> 不过不执行也无所谓。

### step 5

完成阶段性成果，合并到main

```bash
git checkout main
git merge --no-ff dev -m "message" // 同理
git push origin main
```

**step4和step5不要擅自去做，最好商讨之后再进行，或者最起码说一声**

## 3. 关于commit message

禁止空洞的描述（如 "update", "fix"），格式统一为： **`[类型] 简短描述`**

- `[Feature]`：新功能
- `[Fix]`：修补 Bug
- `[Docs]`：仅修改文档/注释
- `[Refactor]`：代码重构（无新功能）
- `[Art]`：美术资源更新
- ...，可以以后再加，这东西以人能看懂为主，不需要那么死板

# 工作留痕日志的规范

## 目的

- 日后Debug方便
- 方便最后整理成项目文档
- 防撞车：防止我们重复造互相的轮子

## 协作平台与结构

共用同一个文档

- 每个功能性分支开一个一级标题，下面记录
    - 在功能性分支的一级标题下面，每个commit开一个二级标题

- 每次merge到dev/main开一个标题，作为记录

## 记录时机

- 每次执行完 Git 协作规范的Step3（即代码成功合并到 `dev` 并推送）之后
- 遇到很难解决的bug，记录解决思路与踩坑点

## 日志规范

例子：

> 2025-11-24 18:59:21: [Feature] 增加了角色的冲刺功能，现在支持按住shift冲刺    \#这个就是commit message
>
> Commit Hash： 0xeeeeeeee （前7位就行）
>
> \# 以下是详细说明
>
> 按住shift之后，角色的移速变为正常步行的2倍，同时消耗体力值每秒10点，动作调用xxx库
>
> **最好附上运行起来的截图，如果有的话**

# Coding Style的规范

**统一使用Google Coding Style。**

如果你不确定你有没有使用正确的，不妨直接让copilot帮你改）

**注释也同样遵循Google Coding Style的规范**

