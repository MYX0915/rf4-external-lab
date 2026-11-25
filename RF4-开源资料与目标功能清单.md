# Russian Fishing 4 — 开源资料调研与目标功能清单

> 调研时间：2026-11-26
> 项目路径：`E:\Users\MYX\Desktop\Russian Fishing 4`
> 状态：**调研阶段，尚未开始实现**

---

## 1. 游戏基本信息

| 项目 | 值 | 来源 |
|---|---|---|
| 进程名 | **`rf4_x64.exe`** | RF4-DSen |
| 引擎 | **Unity + IL2CPP** | RF4-DSen（读 `GameAssembly.dll`） |
| 关键模块 | `GameAssembly.dll`、`global-metadata.dat` | 标准 IL2CPP 布局 |
| 已知版本 | **4.0.25029** | RF4-DSen V1.08 验证目标 |
| 反作弊 | 未见独立反作弊驱动；RF4-DSen 直接用 `OpenProcess` + `ReadProcessMemory` 且正常工作 | 实测仓库行为 |
| 当前本机状态 | **未安装**（未在常见 Steam 路径找到） | 本机检查 |

**重要**：RF4 用 IL2CPP，意味着我们不需要像 Apex 那样逆向崩溃式找偏移——
只要拿到 `GameAssembly.dll` + `global-metadata.dat`，用 **Il2CppDumper** 就能导出
全部类名、字段名、字段偏移、方法地址。

---

## 2. 调研渠道说明

| 渠道 | 状态 | 说明 |
|---|---|---|
| **UnknownCheats** | ❌ 不可访问 | 站点有 Cloudflare challenge，`search.php` 返回 HTTP 403 `challenge_required` |
| **GitHub** | ✅ 可用 | 通过 API 搜索，共 47+ 相关仓库 |
| **grep.app**（GitHub 代码搜索） | ⚠️ 限流 | 返回 HTTP 429 |
| Bing/DuckDuckGo | ❌ 无效 | 中文站忽略查询词；ddg 直连失败 |

⇒ **本次结论全部来自 GitHub 公开仓库**。UC 上的帖子内容无法直接抓取，
如需 UC 内容必须手动登录后检索（建议关键词：`Russian Fishing 4`、`RF4`、`rf4_x64`）。

---

## 3. 开源仓库总览

### 3.1 内存 / 逆向类（**我们的参考重点**）

| 仓库 | 语言 | 星 | 更新 | 关键内容 |
|---|---|---|---|---|
| **`EISEN0516/RF4-DSen`** | Python | 2 | 2026-08 | ⭐ **动态 IL2CPP 读取器**，最完整的参考实现 |
| `luyikk/RussianFishing4Reverse` | C# | 11 | 2020-01 | 含 **`Assembly-CSharp.dll`（10.4 MB）** — 游戏托管程序集 |
| `l963414376/RussianFishing4` | Python | 2 | 2022-03 | 俄钓4辅助工具（模板匹配为主） |
| `nelosss/Russian-fishing-4-data-syphon` | Python | 1 | 2025-04 | 数据采集/上鱼日志（含天气模板图） |
| `hurfy/rf4-api` | Python | 1 | 2025-01 | 非官方渔获 API |

### 3.2 屏幕自动化类（不读内存，纯 CV + 输入模拟）

| 仓库 | 语言 | 星 | 更新 | 特点 |
|---|---|---|---|---|
| **`dereklee0310/RussianFishing4Script`** | Python | **153** | 2026-05 | ⭐ 最成熟，支持路亚/水底/海钓/浮漂四种模式 |
| `Binshifter/RussianFishing4Script` | Python | 2 | 2026-07 | 中文，含 AI 判鱼（火山引擎）、重登卖鱼 |
| `smluntan/RussianFishing4-Tool` | Python | 9 | 2026-06 | 搬砖脚本，含自动化点位截图库 |
| `CoderMzx/RussianFishing4-Script` | Python | 7 | 2025-10 | 中文全自动 |
| `lzsforge/RussianFishing4-Script` | Python | 5 | 2026-06 | 全自动 |
| `Champ10nRR4/RussianFishing4Bot` | C# | 0 | 2024-11 | 钓鱼 + 制作饮料 |
| `e3e0261f/RF4BOT` | AHK | 1 | 2023-05 | AutoHotkey 脚本 |
| `3307129619/RussianFishing4Python` | Python | 5 | 2022-06 | 早期版本 |
| `yao-yi-wei/Russian_fishing_4` | Java | 4 | 2022-06 | 放手脚本 |

### 3.3 数据 / 辅助工具类（无侵入，可复用其数据）

| 仓库 | 语言 | 星 | 内容 |
|---|---|---|---|
| `KoleRF4/castmark` | CSS | 3 | 地图与钓点追踪 |
| `Nalathan01/rf4-companion` | C# | 2 | 目标鱼/钓点/饵料/周记录 + 截图 |
| `TochkiRR4/russian-fishing4` | HTML | 0 | 俄文钓点库 |
| `FliXis24/Russian-Fishing-4-Offline-Maps` | C++ | 7 | 离线地图 |
| `Turyncou/RussianFishing4DataGovernance` | Python | 2 | 数据治理 |
| `Binshifter/RussianFishing4-ChineseLanguagePatch` | — | 0 | 客户端汉化 |

### 3.4 ⚠️ 垃圾仓库警告

2026-11-22 前后集中出现一批 **SEO 垃圾仓库**，描述华丽（"Trophy Fish ESP"、"aimbot and ESP"）
但**没有任何实际代码**（`language: null`，文件仅几 KB，全是 README 堆关键词）：

```
Hunternepinpoint/rf4-bait-lab
riverscorpiontrellis/rf4-tackle-builder
MorningKnight/rf4-trophy-hunter          ← 描述里写 "Trophy Fish ESP"，最容易上当
mrhi21/get-russian-fishing-4-chicory     ← 写 "HWID spoofer, aimbot and ESP"
```

**判定依据**：创建时间集中在同一天、无语言标记、star 数异常整齐（7~9）、
文件数少且全是 txt/md。**不要参考这些仓库。**

---

## 4. 重点仓库详解

### 4.1 `EISEN0516/RF4-DSen` —— 唯一的内存读取实现（核心参考）

一个"上鱼提醒器"HUD，纯 Python，**只读**，不发按键。

**文件构成**

```
rf4_direct_source.py              117 KB   直接内存数据源
app_v104.py                       109 KB   HUD 界面
rf4_thresholds.py                  36 KB   判定阈值
rf4_il2cpp_dynamic_reader.py       29 KB   ⭐ IL2CPP 动态布局读取器
test_il2cpp_dynamic_reader.py       6 KB   回归测试
```

**技术栈（README 原文提炼）**

```
进程：只查找 rf4_x64.exe
读写：OpenProcess + ReadProcessMemory（只读）
识别：GameAssembly.dll 指纹 + IL2CPP class cache 识别当前运行布局
```

**IL2CPP 常量表（从源码提取，可直接复用）**

```python
IL2CPP_CLASS_NAME          = 0x10    # Il2CppClass -> char* name
IL2CPP_CLASS_NAMESPACE     = 0x18    # Il2CppClass -> char* namespace
IL2CPP_CLASS_STATIC_FIELDS = 0xB8    # Il2CppClass -> void* static_fields

LIST_ITEMS   = 0x10                  # List<T> -> T[] items
LIST_SIZE    = 0x18                  # List<T> -> int  size

ARRAY_LENGTH    = 0x18               # Il2CppArray -> uint length
ARRAY_FIRST_ITEM = 0x20              # Il2CppArray -> T[0]
```

**数据链（核心范式）**

```
Il2CppClass(Manager)
   + 0xB8  → static_fields
      + <Singleton 字段偏移>  → 管理器实例

管理器实例
   + <List<Rig> 字段偏移>  → List<Rig>
      + 0x10 → items 数组
      + 0x18 → size
         items + 0x20 + i*8  → Rig 实例
```

**涉及的类与字段**

| 类 | 别名变体 | 用途 |
|---|---|---|
| `FishingSet` | — | 当前渔具组合（1 组 = 3 根竿） |
| `Rig` | `RigBottomSimple`、`RigBottomBase` | 单根鱼竿装配 |
| `RodRestController` | — | 竿的物理位置/朝向（用于 U1/U2/U3 槽位映射） |
| `Fisher` | — | 玩家对象，用于发现当前 live `FishingSet` |

| 字段 | 别名变体 | 含义 |
|---|---|---|
| `rig_in_water` | `inWater` / `isInWater` / `_inWater` | 竿是否在水中 |
| `fight_initialized` | `isFightInitialized` / `_fightInitialized` | 是否进入搏鱼状态 |
| `fight_factor` | `fightFactor` / `_fightFactor` | 搏鱼进度系数 |
| `instance` | `_instance` / `singleton` / `s_instance` | 静态单例字段 |

**咬钩判定条件（原文）**

```
rig_in_water && fight_initialized && fight_factor > 0.0
```

**工程经验（很有价值）**

```
1. 类名被轻度混淆 → 不硬编码字段偏移，改为「语义指纹匹配」：
   · 静态自引用 Singleton
   · List<Rig> 类型
   · 字段名模糊匹配
   · 候选不唯一时报错停止，绝不按顺序猜
2. 依赖 Il2CppDumper 的 script.json（类型/方法/字符串地址）
   + dump.cs（字段 Offset）
3. 标准 script.json 不含实例字段 Offset，必须配 dump.cs
4. 扫描策略：先扫 live 小内存区，避免 70 秒全堆扫描
5. 绝不主动发 1/2/3 热键，避免读数动作改变游戏状态
```

**已实现功能（可作为我们的阶段目标）**

```
✅ 上鱼（咬钩）实时提醒
✅ 鱼名显示（有鱼时优先显示）
✅ 鱼重量显示（未读到显示 --，不显示 0g）
✅ 三根竿槽位映射 U1/U2/U3（用 RodRestController 坐标共识，不用地址顺序）
✅ 换竿/重抛后自动重新发现 FishingSet
✅ 采集诊断包（diagnostics.zip）
❌ 不判断敌我/等级（游戏本身没有这个概念）
```

---

### 4.2 `luyikk/RussianFishing4Reverse` —— 含完整托管程序集

**仓库里有 `Assembly-CSharp.dll`（10.4 MB）**。

- 2020 年的资产，那时 RF4 可能还是 Mono 后端
- 现在已是 IL2CPP，**这个 DLL 的字段偏移不能直接用**
- 但 **类结构、字段命名、继承关系** 仍然极具参考价值
- 可用 ILSpy / dnSpy 反编译，得到全部类图

> 本地下载 10.4 MB 超时未完成，需要时再取：
> `https://raw.githubusercontent.com/luyikk/RussianFishing4Reverse/master/Assembly-CSharp.dll`

---

### 4.3 `dereklee0310/RussianFishing4Script`（153 星，最成熟的自动化）

**纯屏幕方案**（OpenCV 模板匹配 + 键鼠模拟），不读内存。

**功能表（README 原文）**

| 功能 | 说明 |
|---|---|
| Fishing Bot | 自动钓鱼，支持 **路亚 / 水底 / 海钓 / 浮漂** 四种模式 |
| Craft Items | 自动制作饵料、窝料、拟饵 |
| Moving Forward | 自动前进（`W` / `Shift+W` 冲刺） |
| Harvest Baits | 挂机自动采集饵料 |
| **Auto Friction Brake** | 自动调整摩擦刹车 |
| **Calculate Tackle's Stats** | 计算钓组强度与应设的摩擦刹车值 |

**技术要点**

```
收线检测：监控线杯（红框区域）像素；装了彩虹线可用 -R 改用米数（绿框）
状态栏读取：模板匹配 + 阈值（BOT.SPOOL_CONFIDENCE 可调）
窗口要求：窗口化 / 无边框窗口，界面缩放 1x
多竿：只有水底模式支持多竿（1~3 快捷键）
```

---

### 4.4 `Binshifter/RussianFishing4Script`（中文，功能最全的搬砖脚本）

从 README 提炼的功能清单：

```
✅ 全自动钓鱼（平收 / JIG / 海钓三种模式）
✅ 自动进食（定时按 4 键）
✅ AI 判鱼（对接火山引擎，按达标重量筛选留/放）
✅ 自动装配引线/拟饵（模板文件夹扫描生成下拉列表）
✅ 重登回城卖鱼（录制的路线脚本：钓鱼点 ↔ 卖鱼点）
✅ 大鱼超时自动重登（连续 N 次收线超时，防止卡住/磨损）
✅ 时间策略（防检测，建议每日 ≤12 小时）
✅ 日志记录（rf4_fishing.log）
```

**其 `rf4_assembly.py` 实际是模板匹配 + 点击**（名字有误导性），核心逻辑：

```
screen_capture()          → mss 截图转灰度
find_and_click_template() → cv2.matchTemplate(TM_CCOEFF_NORMED) → minMaxLoc → 点击
输入模拟                  → input_utils（human_type_text / key_press / mouse_up）
反检测                    → 随机等待 ±15%、点击偏移 ±2px
```

---

## 5. 功能全景对照表（我们要做什么）

把上面所有仓库的功能合并，RF4 这个游戏能被"做"的功能分三类：

### A. 只读信息类（**内存路线，风险最低，价值最高**）

| 功能 | 实现方式 | 已有实现 |
|---|---|---|
| 咬钩提醒 | 读 `rig_in_water / fight_initialized / fight_factor` | RF4-DSen ✅ |
| 鱼名显示 | 读 `FishingSet` 里的鱼种字段 | RF4-DSen ✅ |
| 鱼重量显示 | 读重量字段 | RF4-DSen ✅ |
| **鱼种/重量/等级预判（上鱼前）** | 读 GPU/咬钩参数 | ❌ 无人做 |
| **鱼的实时坐标/深度** | 读竿/线的物理对象 | ❌ 无人做 |
| **鱼群位置（水下探鱼）** | 待验证游戏是否在客户端保存鱼群对象 | ❌ 无人做 |
| 体力/饥饿/温度 | 读玩家对象字段 | ❌ 无人做 |
| 背包/仓库物品枚举 | 读 Inventory 容器 | ❌ 无人做（只有屏幕 OCR） |
| 当前地图/坐标/朝向 | 读玩家 Transform | ❌ 无人做 |

### B. 自动化操作类（输入模拟，已有成熟实现）

| 功能 | 已有实现 |
|---|---|
| 自动抛竿/收线/提竿 | dereklee、Binshifter、smluntan |
| 自动摩擦刹车 | dereklee ✅ |
| 自动装配引线/拟饵 | Binshifter ✅ |
| 自动进食 | Binshifter ✅ |
| 自动制作饵料 | dereklee ✅ |
| 自动卖鱼/回城 | Binshifter（录制路线）✅ |
| 自动采集饵料 | dereklee ✅ |
| 大鱼超时重登 | Binshifter ✅ |

### C. 数据/可视化类

| 功能 | 已有实现 |
|---|---|
| 钓点库 / 地图 | castmark、rf4-companion、TochkiRR4 |
| 渔获日志与统计 | nelosss、dereklee |
| 天气识别 | nelosss（模板图） |
| 世界纪录对比 | rf4-trophy-hunter（垃圾仓库，只有描述） |

---

## 6. 我们的目标功能（建议分阶段）

### 阶段 0 — 环境准备（必须先做）

```
[ ] 确认游戏已安装，记录版本号与 rf4_x64.exe 路径
[ ] 用 Il2CppDumper 导出 GameAssembly.dll + global-metadata.dat
      → 得到 dump.cs（含全部类/字段/偏移）
      → 得到 script.json（类型地址）
[ ] 建立偏移表 refs/rf4-offsets.txt
[ ] 写一个最小验证：OpenProcess + 读 GameAssembly 基址 + 读一个已知字段
```

### 阶段 1 — 只读信息 HUD（对齐 RF4-DSen）

```
[ ] 定位 Manager 单例（Il2CppClass + 0xB8 → static_fields）
[ ] 找到 FishingSet / Rig / RodRestController / Fisher 类
[ ] 复现咬钩判定：rig_in_water && fight_initialized && fight_factor > 0
[ ] 显示：竿数、竿槽位、咬钩状态、鱼名、重量
[ ] 我们自己加：体力/饥饿/温度/当前地图/坐标
```

### 阶段 2 — 超越现有实现（**这是我们相对开源的优势**）

```
[ ] 上鱼前的鱼种预判（读咬钩前的参数）
[ ] 鱼的实际坐标 / 水深 / 距离（若客户端有物理对象）
[ ] 背包与仓库物品枚举（替代 OCR）
[ ] 竿线实际张力/磨损实时读数（比看 UI 精确）
```

### 阶段 3 — 自动化增强（复用开源思路，自研实现）

```
[ ] 自动抛竿/收线（模板匹配或读内存状态机）
[ ] 自动摩擦刹车（读张力 → 反算最优刹车值，比 CV 精准）
[ ] 自动装配（读内存判断缺件 → 自动补）
[ ] 防检测：随机化、时间策略
```

### 阶段 4 — 可视化

```
[ ] 透明覆盖层 HUD（复用 Apex 项目的 tkinter/GDI 覆盖层经验）
[ ] 钓点/鱼群标记（如果有世界坐标）
[ ] 渔获统计面板
```

---

## 7. 技术要点与坑

### 7.1 IL2CPP 读取的标准范式

```
1. 找 GameAssembly.dll 基址（模块枚举）
2. 从 script.json 拿 Il2CppClass* 的 RVA
3. 类 + 0xB8 → static_fields → 单例
4. 单例 + 字段偏移 → List<T>
5. List + 0x10(items) / 0x18(size)
6. items + 0x20 + i*8 → 元素实例
```

### 7.2 必须注意

```
· 字段偏移随游戏版本变化 → 每次更新都要重新 dump
· 类名可能被混淆 → 用语义指纹而不是硬编码偏移（RF4-DSen 的做法）
· 扫描要增量、分区域，避免全堆扫描卡住游戏
· 只读最安全；一旦写内存要格外小心（封号风险）
· RF4 是网游，服务端校验存在，写内存大概率被发现
```

### 7.3 我们相对 Apex 项目的优势

```
Apex：IL2CPP + EAC 驱动级反作弊 + PVS 裁剪 → 极难
RF4 ：IL2CPP + 未见强反作弊（RF4-DSen 公开用 ReadProcessMemory 且长期可用）
      → 偏移可从 dump.cs 直接得到，不需要逆向
```

### 7.4 风险提示

```
· RF4 是付费网游，账号价值高
· 屏幕自动化已有大量公开脚本（dereklee 153 星长期存活）
· 内存读取（只读）风险低于写内存
· 但任何第三方工具都可能被检测，需要自己承担
```

---

## 8. 待确认问题

```
[ ] 游戏是否已安装？版本号是多少？（RF4-DSen 验证的是 4.0.25029）
[ ] 是否有反作弊？进程里有没有额外的保护模块？
[ ] 目标是什么：只读信息 HUD / 还是带自动化操作？
[ ] 是否需要复用 Apex 项目的 C 内核驱动，还是先用用户态 ReadProcessMemory？
    （RF4-DSen 证明用户态就够，不需要驱动）
```

---

## 9. 参考链接

**核心参考**

- RF4-DSen（IL2CPP 读取器）：https://github.com/EISEN0516/RF4-DSen
- RussianFishing4Reverse（含 Assembly-CSharp.dll）：https://github.com/luyikk/RussianFishing4Reverse

**自动化参考**

- RF4S（153 星）：https://github.com/dereklee0310/RussianFishing4Script
- Binshifter（中文，功能最全）：https://github.com/Binshifter/RussianFishing4Script
- smluntan（搬砖）：https://github.com/smluntan/RussianFishing4-Tool

**数据/工具**

- castmark（钓点）：https://github.com/KoleRF4/castmark
- rf4-api：https://github.com/hurfy/rf4-api
- rf4-companion：https://github.com/Nalathan01/rf4-companion

**必要工具**

- Il2CppDumper：https://github.com/Perfare/Il2CppDumper
- ILSpy（反编译 Assembly-CSharp.dll）：https://github.com/icsharpcode/ILSpy

---

## 10. 本地已保存的参考资料

```
refs/
  RF4-DSen_README.md              4.2 KB    IL2CPP 读取器说明（技术路线全文）
  RF4-DSen_il2cpp_reader.py      28.9 KB    ⭐ IL2CPP 动态读取器源码
  RF4-DSen_RELEASE-NOTES.md       0.7 KB
  RF4-DSen_PACKAGE-NOTES.txt      2.5 KB
  dereklee_README.md              6.5 KB    153 星自动化脚本功能说明
```


---

# 第二部分：社区功能汇总 + 最新 SDK 分析

---

## 11. 社区已实现功能汇总（用户整理，全部是他人已跑通上线/发布的功能）

> 前提：游戏是 Unity IL2CPP，方案来自 UC、GitHub、cheat 论坛、CE 脚本。
> **外部只读 = 仅读内存，不注入 DLL**；注入类风险更高。

### 11.1 外部只读方案（与 BYOVD 外部驱动架构匹配，社区最多人做）

| # | 功能 | 说明 | 落地情况 |
|---|---|---|---|
| 1 | **鱼竿拉力监视器** | 读当前拉力、最大安全拉力、鱼线磨损；Overlay 画拉力条，颜色预警断线 | UC 大量成品、CE 脚本 |
| 2 | **水下鱼群点位标记（轻量鱼 ESP）** | 读 FishManager 里轻量 AI 鱼实体，Overlay 画圆点/包围框；**咬钩前就能看到，但识别不了鱼种/重量** | 多人实现 |
| 3 | **咬口提示 + 奖杯检测** | 监听咬口 flag；触发后读战斗鱼实例，弹提示：鱼种、重量、是否奖杯 | UC 热门 |
| 4 | **假饵/鱼钩坐标 ESP** | 读鱼钩、铅坠、假饵世界坐标；显示饵离水底高度，用来找坎、坑、水草藏鱼点 | CE/外部 ESP |
| 5 | **背包 & 装备读取** | 背包所有饵料、引线、鱼钩库存；当前装备：竿、轮、引线、耐久 | 简单，很多人做 |
| 6 | **玩家状态读取** | 坐标、等级、银币、经验、技能点、饥饿疲劳 | 基础功能 |
| 7 | **游戏环境信息** | 游戏内时间、水温、气压（决定鱼活性），Overlay 展示 | 已有实现 |
| 8 | **点位自动记录器** | 读鱼群坐标自动存本地，形成钓点数据库，下次直接标热点 | 社区脚本 |

### 11.2 内存写入方案（官服风险明显升高，但他人有成品）

| # | 功能 | 说明 |
|---|---|---|
| 1 | 鱼线/鱼竿耐久锁定 | 锁磨损值，永远不会坏 |
| 2 | 搏鱼拉力锁 | 固定鱼线拉力，不超过断线阈值 |
| 3 | 无限投掷饵 | 饵料数量消耗后不减少 |
| 4 | 金钱/经验修改 | 改银币、经验值 |

### 11.3 自动化方案（输入注入，可复用 Apex 项目的鼠标注入经验）

| # | 功能 | 说明 |
|---|---|---|
| 1 | 自动抛投 | 读目标鱼坐标 → 自动瞄准 → 控制力度抛饵 |
| 2 | 自动刺鱼 | 检测咬口事件 → 模拟左键刺鱼 |
| 3 | 自动收线/泄力控制 | 监测拉力，过高自动松泄力防断线，自动遛鱼 |
| 4 | 过滤式自动垂钓 | 只在奖杯/目标鱼种咬口时收线，小鱼直接忽略 |

### 11.4 注入 DLL 专属（外部驱动做不到，高风险）

| # | 功能 | 说明 |
|---|---|---|
| 1 | 原生模型 Outline 描边 | 给鱼 GameObject 挂 Outline 组件，真 3D 轮廓（不是 Overlay 2D 框） |
| 2 | 镜头缩放 | 改相机 FOV，放大水下视野 |
| 3 | 水体透明 | 改水材质渲染参数，直接看透水底 |

### 11.5 难度分层（社区共识）

```
简单（半天~1天）  鱼竿拉力条、背包读取、玩家信息、时间水温读取
中等（1~3天）     鱼钩ESP、鱼群点位标记、咬口奖杯提示、钓点记录
高难度（3~7天）   拉力锁定、耐久锁定
复杂（一周+）     自动抛投、自动遛鱼全套自动化
注入专属          原生鱼描边、透明水体、FOV修改
```

### 11.6 关键社区共识（别人踩完的坑）

```
1. 轻量游动鱼实体 与 咬钩战斗鱼实体，是两套完全独立对象，不能混用
2. IL2CPP + Beebyte 加密，没有长期维护的全局偏移表，偏移随补丁失效，
   优先特征码扫描 / 语义定位
3. 外部驱动无法实现原生 3D 模型描边，只能 Overlay 2D 框/圆点伪描边
```

---

## 12. 最新 SDK 分析（2026-11-08 dump，已解压到 `sdk/`）

### 12.1 这是什么

一份 **UnityDumper 增强版**的完整 IL2CPP 导出，不是普通 Il2CppDumper 输出。
它自带了玩法索引、指针链、CE 表、可调用方法清单、C++ SDK。

```
文件: rf4_x64_20261108_200736_[unknowncheats.me]_.zip  (25.19 MB)
解压: sdk/rf4_x64_20261108_200736/                     (407 MB, 154 个文件)
```

### 12.2 DumpManifest 关键信息（原文）

```
Game       : rf4_x64
Runtime    : IL2CPP  (D:\SteamLibrary\steamapps\common\RussianFishing4\GameAssembly.dll)
Unity      : 2022.3.62f3 (96770f904ca7)
Base       : 0x7FFEEBC50000   (当次 dump 时的模块基址，重启会变)
Assemblies : 90 | classes 19025 (game 9185) | generic instances 13473
Warnings   : 0 | errors 0
```

**注意**：dump 者的游戏装在 `D:\SteamLibrary\steamapps\common\RussianFishing4\`
（本机未安装到这个路径，需要确认自己的安装位置）

### 12.3 Health.txt 全部 PASS（这份 dump 的可信度很高）

```
[1] RUNTIME
  base / size    : 0x7FFEEBC50000 / 0x4855000    (GameAssembly.dll 约 75.8 MB)
  Unity version  : 2022.3.62f3
  pointer size   : 8 (object header 0x10)
  metadata       : global-metadata.dat 不在标准路径（从内存读的）

[2] EXPORTS       required 15 / optional 36，全部解析
[3] COLLECTION    classes 19025 / fields 91144 / methods 131529 / properties 28477
[6] IL2CPP DEPTH  Il2CppClass::static_fields at +0xB8  (实测确认)
                  11254 个 TypeInfo 槽 / 5889 个 MethodInfo 槽 / 5951 个字符串字面量槽

RESULT: PASS   (25 ok, 0 warnings, 1 n/a)
```

### 12.4 目录结构（哪些文件有用）

| 路径 | 大小 | 用途 |
|---|---|---|
| `dump.cs` | 26 MB | ⭐ 全部类/字段/方法/偏移（425575 行） |
| `offsets.json` | 129 MB | 完整偏移 JSON |
| `GameplayIndex/GameplayIndex.txt` | 909 KB | ⭐ 单例、玩家、ESP 入口索引（12615 行） |
| `PointerPaths/PointerPaths.txt` | 45 KB | ⭐ 静态指针链 |
| `PointerPaths/UnityDumper.CT` | 128 KB | ⭐ Cheat Engine 表，可直接加载 |
| `CallableMethods/CallableMethods.txt` | 3.4 MB | 值得调用的游戏方法 + 调用配方 |
| `CppSDK/*.hpp` | 38 个 | 带类型的 C++ SDK（`SDK.hpp` 是入口） |
| `Il2CppHeader/script.json` | 32 MB | 类型/方法地址（给 IDA/Ghidra/Binja） |
| `Il2CppHeader/il2cpp.h` | 30 MB | 结构体定义 |
| `StringLiterals/` | — | 全部字符串字面量（找 UI 文本、鱼名用） |
| `WorldGraph/WorldGraph.txt` | 36 KB | 场景对象关系图 |
| `Snapshot/Members.tsv` | 17 MB | 全成员快照（用于版本 diff） |
| `Scripts/*_names.py` | 22 MB × 3 | IDA/Ghidra/Binja 自动命名脚本 |
| `Scripts/x64dbg_labels.dd64` | 14 MB | x64dbg 标签 |

> **这份 dump 直接把我们从"逆向找偏移"降级成"查表"**。
> 之前 Apex 项目花了几天的活，这里是一份文件的事。

---

## 13. 关键类与字段（从 dump.cs 实测提取）

### 13.1 IL2CPP 运行时常量（Health.txt 实测 + RF4-DSen 交叉验证）

```python
# Il2CppClass 布局
IL2CPP_CLASS_NAME          = 0x10     # char* name
IL2CPP_CLASS_NAMESPACE     = 0x18     # char* namespace
IL2CPP_CLASS_STATIC_FIELDS = 0xB8     # void* static_fields   ← 实测确认

# 容器
LIST_ITEMS   = 0x10     # List<T> -> T[] items
LIST_SIZE    = 0x18     # List<T> -> int size
ARRAY_LENGTH = 0x18     # Il2CppArray -> uint length
ARRAY_FIRST  = 0x20     # Il2CppArray -> T[0]

# 对象
OBJECT_HEADER = 0x10    # 所有 Unity 对象头
UnityEngine.Object.m_CachedPtr = 0x10
System.String: length 0x10, chars 0x14
```

### 13.2 `RF4.Client.GameController`（主入口单例）

```csharp
internal class RF4.Client.GameController : ...
{
    private static RF4.Client.GameController ellmppcbkao;   // static 0x0  ← 单例
    private static readonly iaikbaoignh dpakaefemlc;        // static 0x8
    protected RF4.Client.UIController m_ui;                 // 0x20
    protected RF4.Client.Sound.GameSoundController m_sound; // 0x28
    private nnfmkefpbdi.nfngambecpd ihgcehcapci;            // 0x30
    protected RF4.Client.GameLevelLoader m_levelLoader;     // 0x38
}
```

### 13.3 ⭐ `RF4.Client.MyBoid`（**游动鱼实体 —— 社区说的"轻量鱼 ESP"就在这里**）

```csharp
public static List<RF4.Client.MyBoid> adaoecddedc;          // static 0x0  ← 全部游动鱼
public static List<RF4.Client.BeaconGizmo> bocpjpmlkll;     // static 0x8  ← 信标

public System.Boolean visibleByCamera;   // 0x58   是否在相机视野内
public UnityEngine.Animator dbkgphpoajp; // 0x70
private UnityEngine.Vector3 amdldkbppia; // 0x78
protected UnityEngine.Vector3 jamcobfgbei;  // 0x84
protected UnityEngine.Vector3 bioejnpeahk;  // 0x90
protected UnityEngine.Quaternion aaaafokihad; // 0x9C
protected UnityEngine.Quaternion molbldgmhka; // 0xAC
public UnityEngine.Transform tr;         // 0xC0   ← Transform，直接取 world position
public UnityEngine.Vector3 velocity;     // 0xC8   ← 游动速度/方向
public ilflkbbmbmp species;              // 0xD4   ← 鱼种类别（枚举，3 个值）
public fabgpofmbif state;                // 0xD8   ← 状态（枚举，5 个值）
protected RF4.Client.BeaconGizmo llhgpjdpdin; // 0xE0
public System.Single _maxSteerForce;     // 0xE8
public System.Single _maxSpeed;          // 0xEC
public System.Single _minSpeed;          // 0xF0

// 活动边界（鱼群游动范围）
public System.Single minX; // 0x30   maxX; // 0x34
public System.Single minZ; // 0x38   maxZ; // 0x3C
public System.Single minY; // 0x40   maxY; // 0x44
```

**这意味着**：用户提到的功能 #2「水下鱼群点位标记」**完全可做**，
而且 `species` + `state` 字段让"识别鱼种类别/状态"也有了入口
（社区说"识别不了鱼种"——他们可能没读这两个字段）。

### 13.4 ⭐ `RF4.Client.FishingScene.RodRestController`（竿位，U1/U2/U3 映射靠它）

```csharp
internal class RodRestController : UnityEngine.MonoBehaviour
{
    protected oljcomcodha manndckkhde;                    // 0x20
    private UnityEngine.Quaternion <gagomngadgc>k__BackingField; // 0x28  ← 朝向
    private System.Action<...> gombcndkpgo;               // 0x38
    private UnityEngine.Vector3 <lpgfhbclfck>k__BackingField;    // 0x40  ← 世界坐标
    public System.Boolean externalByteAlarm;              // 0x4C
    private System.Boolean <ihglalokcil>k__BackingField;  // 0x4D
    protected UnityEngine.Transform cicmhgacljm;          // 0x50
    private phmgbolhlai <mcinepgnien>k__BackingField;     // 0x58
    public System.Boolean deleteAfterDisconnect;          // 0x60
}
```

**竿的世界坐标 = `RodRestController + 0x40`（Vector3），朝向 = `+0x28`（Quaternion）**

### 13.5 `RF4.Client.FishingScene.Rig`（单根竿装配，含拉力/磨损阈值）

```csharp
internal abstract class Rig : RF4.Client.GameBehaviour, ...
{
    public const System.Single WarningRodTestFactor  = 1.02499998f;  // 预警阈值
    public const System.Single CriticalRodTestFactor = 1.10000002f;  // 危险阈值
    private UnityEngine.Transform ofanjomopko;   // 0x40
    private System.Double <fhgbchhopia>k__BackingField; // 0x48
    protected ebabccgcfjl lmkifikkfkn;           // 0x50
    private System.Single jeolelhmflo;           // 0x58
    protected oljcomcodha maodcphblpk;           // 0x60
    private System.Boolean meghhnalieg;          // 0x68
    protected UnityEngine.Vector2 kjkfofnkeml;   // 0x6C
    private System.Single ddcmomlapma;           // 0x74
    private readonly jgamblffchk gbedcpnnfpo;    // 0x78
    private System.Single alkbhicjjnn;           // 0x80
    private System.Double <dhdlbiaebjo>k__BackingField; // 0x88
    protected effjdeblglh coffdfkgfaa;           // 0x90
    private UnityEngine.Transform jedmopeicbe;   // 0x98
    private System.Single <edikjnhjbmb>k__BackingField; // 0xA8
    private System.Boolean <dlblgjodfha>k__BackingField; // 0xB0
}
```

字段名全被 Beebyte 混淆，但有 **30+ 个 Vector3/Double/Single/Bool 属性**，
需要结合游戏内实际数值逐个确认语义（拉力、磨损、是否在水中、搏鱼状态等）。

### 13.6 类继承链

```
Rig (abstract, : GameBehaviour)
 ├── RodRigFishingSet
 │     └── RodReelRigFishingSet
 └── RigBottomSimple / RigBottomBase（RF4-DSen 提到的别名）

FishingSet (基类)
 └── RodRigFishingSet
       └── RodReelRigFishingSet

FishingSetUserInputController
 └── 字段 set : FishingSet  // 0x40   ← 从输入控制器拿当前 FishingSet
```

### 13.7 `RF4.Client.*` 单例清单（ESP 数据入口）

| 类 | 静态偏移 | 实例字段 | 用途 |
|---|---|---|---|
| **`RF4.Client.GameController`** | static +0x0 | `ellmppcbkao` | ⭐ 主入口 |
| `RF4.Client.FishingScene.FishingZonesManager` | static +0x0 | `<kanghfjgcak>` | 钓区管理 |
| `RF4.Client.Assets.FishAssetCollection` | static +0x0 | `ehjeienekge` | 鱼类资产表（+0x20 → FishFamilyAsset[]） |
| `RF4.Client.Weather.WeatherManager` | static +0x38 | `ellmppcbkao` | ⭐ 天气（时间/水温/气压） |
| `RF4.Client.WaterTextures` | static +0x20 | `jagiidlgefn` | 水面 |
| `RF4.Client.AllLOD` | static +0x0 | `ellmppcbkao` | LOD 管理 |
| `RF4.Client.School.SchoolController` | static +0x0 | `<kanghfjgcak>` | 教学 |
| `RF4.Client.MyBoid` | static +0x0 | `adaoecddedc` : `List<MyBoid>` | ⭐ 全部游动鱼 |
| `RF4.Client.MyBoid` | static +0x8 | `bocpjpmlkll` : `List<BeaconGizmo>` | 信标 |
| `RF4.Client.FishingScene.CampFireWatchDog` | static +0x0 | `ijbogjfoall` | 篝火 |

### 13.8 已提取到本地的文件

```
refs/rf4_key_classes.txt     提取的核心类完整定义
  · FishingSet                 389 行
  · RodRigFishingSet           119 行
  · RodReelRigFishingSet       107 行
  · RodRestController           51 行
  · Fisher                     327 行
  · Rig                        183 行
  · FishingZonesManager         31 行
  · FishAssetCollection         12 行
  · FishFamilyAsset             23 行

tools/extract_classes.py     按类名从 dump.cs 提取定义块的脚本
tools/extract_more.py        提取 GameController / MyBoid / 输入控制器
tools/extract_enums.py       提取枚举
```

---

## 14. 目标功能修订（基于 SDK 能力重新分级）

有了 SDK，"能不能做"的问题全部变成"怎么读"的问题。

### 14.1 立即可做（数据入口已明确，无需逆向）

| 功能 | 数据源 | 难度 |
|---|---|---|
| **水下鱼群 ESP**（圆点/框 + 距离） | `MyBoid.static+0x0` → List → `tr`(0xC0) 世界坐标 | 低 |
| **鱼种/状态标记** | `MyBoid.species`(0xD4) / `state`(0xD8) | 低 |
| **鱼群活动范围可视化** | `MyBoid.minX/maxX/minZ/maxZ/minY/maxY`(0x30~0x44) | 低 |
| **鱼竿拉力监视器** | `Rig` 的 Single/Double 字段（需在游戏内标定语义） | 中 |
| **竿位 U1/U2/U3** | `RodRestController` × N → `+0x40` 坐标共识 | 低 |
| **玩家状态** | `GameController` → 逐层下钻 | 中 |
| **天气/时间/水温** | `WeatherManager static+0x38` | 低 |
| **鱼类资产全表** | `FishAssetCollection static+0x0` → `+0x20` → `FishFamilyAsset[]` | 低 |

### 14.2 需要标定（字段名被混淆，得结合游戏实测确认）

```
Rig 的 30+ 个单/双精度字段 → 哪个是拉力、哪个是磨损、哪个是鱼重
Fisher 的 327 行字段       → 体力、饥饿、疲劳、银币、等级
FishingSet 的 389 行字段   → 当前装配的完整信息
```

**标定方法**：写个 dump 工具，把某个对象的全部字段按偏移导出成快照，
然后在游戏里做单一变化（吃一口东西 / 拉一下鱼 / 卖一条鱼），再导出对比，
**变了的字段就是目标字段**。这比逆向快得多。

### 14.3 需要额外工作

| 功能 | 障碍 | 方案 |
|---|---|---|
| 咬钩战斗鱼实体（鱼名/重量/奖杯） | 与 MyBoid 是**两套独立对象**（社区共识） | 从 `FishingSet → Rig → 战斗鱼引用` 追 |
| 背包物品枚举 | 需找到 Inventory 容器 | 从 `GameController` 下钻 or `CallableMethods.txt` 找 |
| 自动抛投/遛鱼 | 需要世界坐标 → 屏幕坐标（W2S） | SDK 提供 `Camera` + 矩阵，纯 C++ W2S |

### 14.4 我们相对社区的优势

```
1. 有完整 SDK（社区多数人靠特征码硬扫）
2. MyBoid 的 species/state 字段可做「咬钩前识别鱼种」——
   社区共识说「轻量鱼无法识别鱼种」，但他们很可能没读这两个字段
3. 可复用 Apex 项目的覆盖层、输入注入、C 驱动经验
```

### 14.5 技术路线建议（修订版）

```
第一步  用 UnityDumper.CT（CE 表）直接在 CE 里验证 MyBoid 链表
        → 确认 static+0x0 能解析出真实鱼对象
第二步  写最小 Python 读取器（OpenProcess + ReadProcessMemory）
        → 复现「鱼群 ESP」的坐标采集
第三步  复用 Apex 的覆盖层，把鱼群画成圆点 + 距离
第四步  标定 Rig/Fisher 字段（快照对比法）
第五步  按需扩展
```

---

## 15. 更新后的待确认问题

```
[ ] 本机游戏安装路径？（dump 者的是 D:\SteamLibrary\steamapps\common\RussianFishing4）
[ ] 本机游戏版本？是否等于/接近 4.0.25029（RF4-DSen 验证版本）
    和 2026-11-08 dump 的版本
[ ] 目标优先级：先做鱼群 ESP，还是先做拉力监视器？
[ ] 是否需要先做一个 dump/快照工具（用于字段标定）
```


---

## 16. 当前状态快照（2026-11-26 01:23）

### 16.1 已完成

```
[✓] 调研渠道打通（GitHub API 可用；UC 被 Cloudflare 挡，需手动查）
[✓] 47+ 相关仓库筛选，剔除 SEO 垃圾仓库，确定 3 个核心参考
[✓] RF4-DSen 的 IL2CPP 读取器源码已下载并提取常量表
[✓] 最新 SDK（2026-11-08 UnityDumper 导出）已解压并验证 Health PASS
[✓] 从 dump.cs 提取 9 个核心类的完整定义（refs/rf4_key_classes.txt）
[✓] 定位游动鱼实体 MyBoid（含 species/state/Transform）
[✓] 定位竿位控制 RodRestController（坐标 +0x40 / 朝向 +0x28）
[✓] 定位主入口 GameController 单例
[✓] 提取 RF4.Client.* 全部单例清单
[✓] 建立 3 个 dump.cs 提取脚本（tools/）
```

### 16.2 未完成

```
[ ] 确认本机游戏安装路径与版本号
[ ] CE 表（UnityDumper.CT）实测验证 MyBoid 链表
[ ] 写最小读取器（OpenProcess + ReadProcessMemory）
[ ] Rig / Fisher 字段语义标定（Beebyte 混淆，需快照对比法）
[ ] 咬钩战斗鱼实体的数据链（与 MyBoid 是两套对象）
```

### 16.3 本地文件清单

```
E:\Users\MYX\Desktop\Russian Fishing 4\
├── RF4-开源资料与目标功能清单.md          33.6 KB / 821 行 / 16 章   ← 本文档
├── rf4_x64_20261108_200736_[unknowncheats.me]_.zip   25.2 MB  原始 SDK 压缩包
├── sdk/rf4_x64_20261108_200736/         264 MB / 59 文件
│   ├── dump.cs                          26 MB / 425575 行   全部类型
│   ├── offsets.json                    129 MB                全部偏移
│   ├── GameplayIndex/GameplayIndex.txt   909 KB / 12615 行   单例与 ESP 入口
│   ├── PointerPaths/PointerPaths.txt      45 KB              静态指针链
│   ├── PointerPaths/UnityDumper.CT       128 KB              CE 表
│   ├── CallableMethods/CallableMethods.txt 3.4 MB            可调用方法
│   ├── CppSDK/SDK.hpp                    4 KB  入口        带类型的 C++ SDK
│   ├── Il2CppHeader/script.json         32 MB  给 IDA/Ghidra
│   ├── Snapshot/Members.tsv             17 MB  版本 diff 用
│   └── Scripts/*_names.py               22 MB × 3  反编译器命名脚本
├── refs/
│   ├── rf4_key_classes.txt             105 KB  9 个核心类定义
│   ├── RF4-DSen_il2cpp_reader.py        29 KB  IL2CPP 读取器源码
│   ├── RF4-DSen_README.md              4.2 KB  技术路线说明
│   ├── RF4-DSen_RELEASE-NOTES.md       0.7 KB
│   ├── RF4-DSen_PACKAGE-NOTES.txt      2.5 KB
│   └── dereklee_README.md              6.5 KB  153 星自动化脚本功能
└── tools/
    ├── extract_classes.py              2.1 KB  按类名提取定义块
    ├── extract_more.py                 1.2 KB  GameController / MyBoid
    ├── extract_enums.py                1.0 KB  枚举提取
    └── append_doc.py                    17 KB  文档追加
```

### 16.4 建议的第一阶段目标（已定）

**鱼群 ESP —— 链路最短、验证最快、且有差异化**

```
理由：
  1. 数据入口最清晰：一个静态 List<MyBoid>，无需字段标定
  2. 只需 tr(0xC0) 一个坐标字段即可出画
  3. species(0xD4)/state(0xD8) 让「咬钩前识别鱼种」成为可能
     —— 社区共识说做不到，SDK 显示能做，这是我们的差异点
  4. 能一次性验证完整链路（进程 → 类 → 静态字段 → List → 对象 → Transform → W2S）

步骤：
  Step 1  CE 里加载 UnityDumper.CT，手动验证 static MyBoid+0x0 能否解析出真实鱼对象
  Step 2  写最小 Python 读取器复现同样结果
  Step 3  接覆盖层，画圆点 + 距离 + 鱼种/状态标签
  Step 4  扩展到鱼群活动范围（minX/maxX/minY/maxY/minZ/maxZ）
```

### 16.5 技术风险备忘

```
1. dump 是 2026-11-08 的，游戏若已更新，偏移会变
   → 每次更新重跑 UnityDumper（比手写特征码省事）
2. 字段名 Beebyte 混淆，语义靠实测标定，不能靠名字猜
3. 游动鱼(MyBoid) 与 咬钩战斗鱼 是两套对象，别混用
4. 本机游戏路径/版本尚未确认，第一次运行前必须先核对
```

---

## 17. 仓库规划

### 17.1 仓库命名

```
推荐：rf4-external-lab
备选：russian-fishing-4-reverse
      rf4-il2cpp-lab
```

命名逻辑与前一个项目保持一致（`r5-external-esp-lab`）：

```
r5-external-esp-lab       →  Apex 项目（r5 = 进程名前缀）
rf4-external-lab          →  RF4 项目（rf4 = 进程名前缀）
```

### 17.2 仓库简介（GitHub Description）

**英文（推荐，GitHub 上更通用）**

```
Russian Fishing 4 (rf4_x64) reverse engineering lab — IL2CPP offset research, external read-only ESP prototypes, and field-mapping notes. Personal research, not for online use.
```

**中文备选**

```
俄罗斯钓鱼4 (rf4_x64) 逆向研究实验库 —— IL2CPP 偏移研究、外部只读 ESP 原型、字段语义标定记录。
```

### 17.3 建议的 Topics

```
il2cpp  unity  reverse-engineering  game-hacking  offset-dump
russian-fishing-4  rf4  esp  cheat-research  memory-reader
```

### 17.4 建议的仓库结构

```
rf4-external-lab/
├── README.md                      项目说明与进度
├── docs/
│   ├── RF4-开源资料与目标功能清单.md     ← 现有文档搬过来
│   └── offsets/
│       ├── il2cpp-runtime.md      运行时常量表
│       └── key-classes.md         核心类字段表
├── sdk/                           ⚠️ 不建议入库（264 MB，且含第三方 dump）
│   └── .gitkeep
├── refs/                          ⚠️ 第三方源码只放链接，不入库
│   └── LINKS.md
├── tools/
│   ├── extract_classes.py
│   ├── extract_more.py
│   ├── extract_enums.py
│   └── snapshot_fields.py         （待写：字段快照对比工具）
├── src/
│   ├── reader/                    最小内存读取器（待写）
│   └── overlay/                   覆盖层（待写）
└── .gitignore                     sdk/ refs/*.py *.zip *.json *.cs
```

**入库建议**

```
✅ 入库：文档、脚本、我们自己写的代码
❌ 不入库：SDK dump 264 MB、第三方仓库源码、游戏文件
   （.gitignore 里排除，README 里写明获取方式）
```
