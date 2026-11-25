# C1 启动菜单计划

A1 的框架已进入远端 `main`。从最新 `origin/main` 建 `codex/m1-c1`，在 UE 5.8 验证。[C1 #15](https://github.com/HOMG-Dev/HOMG2/issues/15) · [A1 #11](https://github.com/HOMG-Dev/HOMG2/issues/11)

## 实施

1. 创建 `/Game/Maps/M1/L_M1_Menu` 空菜单关卡，配置菜单专用 GameMode 和本地 PlayerController。菜单关卡使用自己的玩家类和 HUD 设置，模板资产保留原样。
2. 创建简洁的全屏菜单 Widget，显示「开始游戏」「退出」两个可聚焦按钮。默认聚焦「开始游戏」，启用 `UI Only` 和鼠标光标，由本地 PlayerController 创建和清理菜单。给两个按钮绑定独立回调，分别记录可验证的点击日志，为 C2 和 C3 留出接入点。
3. 把 `GameDefaultMap` 指向菜单关卡。编译并保存实际用到的地图、控件和配置，可用 MCP 检查资产和当前关卡并执行 PIE 冒烟测试。

## 独立子代理验收

1. 启动无地图覆盖参数的 Standalone 游戏进程。确认默认直接进入 `L_M1_Menu`，没有模板角色或模板 HUD，界面只提供两项操作。
2. 分别用鼠标与键盘触发两个按钮。验证启动时焦点可见、Tab 可切换、Enter 可激活，日志能区分两个回调。重启后再检查初始焦点。
3. 检查地图引用、改动范围和菜单资源清理。留存启动截图、按钮日志与构建结果。

## 本地实施记录

- 基线为 `origin/main` 的 `2e92ac2`。菜单地图指定 `M1MenuGameMode`，由 `M1MenuPlayerController` 创建原生 UMG。`GameDefaultMap` 指向菜单地图。
- UE 5.8.4 的 `HOMG2Editor Win64 Development` 构建成功。构建摘要位于本地 `game/Saved/M1C1/BuildResult.txt`。
- 从项目以 `-game -windowed -ResX=960 -ResY=540` 启动，省略地图参数。启动日志记录 `LoadMap: /Game/Maps/M1/L_M1_Menu` 和 `Game class is 'M1MenuGameMode'`。主实现者先用 Enter、Tab 后的 Enter，再分别点击两个按钮，回调日志依次为 Start、Exit、Start、Exit。

## 独立验收结果

**通过。** 独立验收子代理启动了两次 Standalone，确认只显示两个按钮，重启后「开始游戏」默认高亮。Tab 切换焦点，Enter 和鼠标均可分别激活两个按钮。源码审查确认菜单不会生成模板 Pawn 或 HUD，结束时解除绑定并移除菜单。

- 本地截图为 `game/Saved/M1C1/IndependentStartup.jpg`。
- 首轮四项输入与 `game/Saved/M1C1/IndependentStandalone.log` 第 1854、1857、1859、1865 行逐一对应。此前两条来源未确认的回调未计入验收。第二轮启动后首次按 Enter 的回调见 `game/Saved/M1C1/IndependentRestart.log` 第 1765 行，之前没有菜单动作日志。
