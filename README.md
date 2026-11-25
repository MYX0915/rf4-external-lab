# RF4 External Lab

Russian Fishing 4 (`rf4_x64`) 逆向研究实验库 —— IL2CPP 偏移研究、外部只读 ESP 原型、字段语义标定记录。

> 个人技术研究用途。不注入、不修改内存、不提供线上使用的成品。

---

## 项目定位

RF4 是 Unity **IL2CPP** 游戏（Unity 2022.3.62f3），与 Apex 那类「带内核反作弊 + PVS 裁剪」
的对手完全不同：**一份完整的 IL2CPP dump 就能拿到全部类、字段、偏移**，
不需要崩溃式逆向。本仓库记录这条链路的全部过程与产物。

**目标**：只读、外部、覆盖层呈现——不做写内存，不做注入。

---

## 现状

| 阶段 | 状态 |
|---|---|
| 社区调研（UC / GitHub / 开源仓库） | ✅ 完成 |
| 参考实现分析（RF4-DSen IL2CPP 读取器） | ✅ 完成 |
| SDK dump 分析（2026-11-08 UnityDumper 导出） | ✅ 完成 |
| 核心类字段提取（9 个类） | ✅ 完成 |
| 游动鱼实体定位（`MyBoid`） | ✅ 完成 |
| 本机游戏版本核对 | ⬜ 待做 |
| 最小内存读取器 | ⬜ 待做 |
| 鱼群 ESP 原型 | ⬜ 待做 |

进度细节见 [`RF4-开源资料与目标功能清单.md`](RF4-开源资料与目标功能清单.md)（17 章）。

---

## 关键发现

### 游戏是 IL2CPP，偏移从 dump 查表即可

```
Game       rf4_x64
Runtime    IL2CPP  (GameAssembly.dll)
Unity      2022.3.62f3
Classes    19025 (game 9185) | fields 91144 | methods 131529
Health     PASS (25 ok, 0 warnings)
```

### 运行时常量（实测确认）

```python
Il2CppClass + 0x10  → char* name
Il2CppClass + 0x18  → char* namespace
Il2CppClass + 0xB8  → void* static_fields

List<T> + 0x10      → items        List<T> + 0x18 → size
Il2CppArray + 0x18  → length       Il2CppArray + 0x20 → first item

UnityEngine.Object.m_CachedPtr = 0x10
System.String: length 0x10, chars 0x14
```

### 三个核心对象

```csharp
// 游动鱼实体（水下的鱼群）
RF4.Client.MyBoid
    static +0x0     List<MyBoid> adaoecddedc     ← 全部游动鱼
    +0x58   bool            visibleByCamera
    +0xC0   Transform       tr                   ← 世界坐标
    +0xC8   Vector3         velocity
    +0xD4   ilflkbbmbmp     species              ← 鱼种类别
    +0xD8   fabgpofmbif     state                ← 状态
    +0x30~0x44  float       minX/maxX/minZ/maxZ/minY/maxY   ← 活动范围

// 竿位（U1/U2/U3 映射）
RF4.Client.FishingScene.RodRestController
    +0x28   Quaternion      朝向
    +0x40   Vector3         世界坐标

// 单根竿（拉力/磨损，字段名被 Beebyte 混淆，待标定）
RF4.Client.FishingScene.Rig
    const float WarningRodTestFactor  = 1.025f
    const float CriticalRodTestFactor = 1.10f
```

### 主入口

```csharp
RF4.Client.GameController                  static +0x0
RF4.Client.Weather.WeatherManager          static +0x38   ← 时间/水温/气压
RF4.Client.Assets.FishAssetCollection      static +0x0 → +0x20 → FishFamilyAsset[]
RF4.Client.FishingScene.FishingZonesManager static +0x0
```

---

## 目录

```
.
├── README.md                            本文件
├── RF4-开源资料与目标功能清单.md          主文档（17 章 / 985 行）
├── docs/
│   └── offsets/
│       ├── il2cpp-runtime.md            IL2CPP 运行时常量表
│       └── key-classes.md               核心类字段表
├── refs/
│   └── LINKS.md                         第三方参考仓库与获取方式
├── tools/
│   ├── extract_classes.py               从 dump.cs 按类名提取定义块
│   ├── extract_more.py                  提取 GameController / MyBoid 等
│   └── extract_enums.py                 枚举提取
├── src/                                 （待建）读取器与覆盖层
└── .gitignore
```

### 不入库的内容

| 内容 | 大小 | 说明 |
|---|---|---|
| `sdk/rf4_x64_*` | 264 MB | IL2CPP dump，从 unknowncheats 获取 |
| `*.zip` | 25 MB | dump 原始压缩包 |
| `refs/*.py` 等 | — | 第三方仓库源码，仅本地参考，见 `refs/LINKS.md` |

获取方式见 [`refs/LINKS.md`](refs/LINKS.md)。

---

## 工具用法

```powershell
# 从 dump.cs 提取核心类 → refs/rf4_key_classes.txt
python tools/extract_classes.py

# 提取主入口与鱼实体类（直接打印字段区）
python tools/extract_more.py

# 提取枚举
python tools/extract_enums.py
```

脚本按类名在 `dump.cs` 中定位定义块（配对大括号），
不依赖符号表，任何版本的 dump 都能用。

---

## 参考实现

| 项目 | 说明 |
|---|---|
| [EISEN0516/RF4-DSen](https://github.com/EISEN0516/RF4-DSen) | 唯一公开的 IL2CPP 内存读取器，本项目的技术路线来源 |
| [luyikk/RussianFishing4Reverse](https://github.com/luyikk/RussianFishing4Reverse) | 含 2020 版 `Assembly-CSharp.dll`，可反编译看类结构 |
| [dereklee0310/RussianFishing4Script](https://github.com/dereklee0310/RussianFishing4Script) | 153 星屏幕自动化，四种钓法 |
| [Binshifter/RussianFishing4Script](https://github.com/Binshifter/RussianFishing4Script) | 中文搬砖脚本，功能最全 |

完整清单见主文档第 3 节。

---

## 风险说明

```
· 本项目只做只读研究，不写内存、不注入
· RF4 是付费网游，任何第三方工具都存在检测风险
· 文档中记录的偏移来自特定版本 dump，游戏更新后需要重新导出
· 与 Apex 项目不同：这里没有内核驱动，用户态 ReadProcessMemory 即可
```

---

## 许可

个人研究用途。文档与自写脚本可自由参考；
第三方 dump 与源码的版权归各自作者，请遵守其原始许可。
