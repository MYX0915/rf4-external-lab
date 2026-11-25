# 第三方参考资料

> 本仓库不收录第三方源码与 dump 文件，只记录获取方式与要点。
> 本地下载的文件放在 `refs/`（已被 `.gitignore` 排除）。

---

## 1. IL2CPP dump（必需）

### 1.1 本项目使用的 dump

```
文件    rf4_x64_20261108_200736_[unknowncheats.me]_.zip   (25.2 MB)
解压后  sdk/rf4_x64_20261108_200736/                     (264 MB / 59 文件)
生成器  UnityDumper（增强版 Il2CppDumper）
获取    unknowncheats.me 论坛 RF4 板块
```

**关键信息**

```
Game     rf4_x64
Runtime  IL2CPP
Unity    2022.3.62f3 (96770f904ca7)
Base     0x7FFEEBC50000  （当次 dump，重启会变）
Classes  19025（游戏 9185）
Health   PASS (25 ok, 0 warnings)
```

**有用的文件**

| 路径 | 大小 | 用途 |
|---|---|---|
| `dump.cs` | 26 MB | 全部类型定义（425575 行） |
| `offsets.json` | 129 MB | 完整偏移 JSON |
| `GameplayIndex/GameplayIndex.txt` | 909 KB | 单例 / ESP 入口索引 |
| `PointerPaths/PointerPaths.txt` | 45 KB | 静态指针链 |
| `PointerPaths/UnityDumper.CT` | 128 KB | Cheat Engine 表，可直接加载 |
| `CallableMethods/CallableMethods.txt` | 3.4 MB | 值得调用的方法 + 调用配方 |
| `CppSDK/SDK.hpp` | 4 KB | 带类型的 C++ SDK 入口 |
| `Il2CppHeader/script.json` | 32 MB | 给 IDA / Ghidra / Binja 的类型地址 |
| `Il2CppHeader/il2cpp.h` | 30 MB | 结构体定义 |
| `StringLiterals/StringLiterals.txt` | 232 KB | 字符串字面量（对混淆字段很有用） |
| `Snapshot/Members.tsv` | 17 MB | 版本 diff 用 |
| `Scripts/*_names.py` | 22 MB × 3 | 反编译器自动命名脚本 |
| `Scripts/x64dbg_labels.dd64` | 14 MB | x64dbg 标签 |

### 1.2 自己重新导出

```powershell
# 标准 Il2CppDumper
Il2CppDumper.exe GameAssembly.dll global-metadata.dat 输出目录

# 文件位置（Steam 版）
<游戏目录>\GameAssembly.dll
<游戏目录>\rf4_x64_Data\il2cpp_data\Metadata\global-metadata.dat
```

- [Il2CppDumper](https://github.com/Perfare/Il2CppDumper) —— 标准导出，得到 `dump.cs` + `script.json`
- 增强版（UnityDumper）额外产出 GameplayIndex / PointerPaths / CppSDK / Snapshot，需自行寻找

> ⚠️ 游戏更新后偏移会变，每次大版本更新都要重新导出。

---

## 2. 核心参考实现

### 2.1 RF4-DSen（IL2CPP 读取器，**本项目技术路线来源**）

```
https://github.com/EISEN0516/RF4-DSen
Python / 2★ / 更新 2026-08
```

**亮点**

- 唯一公开的 RF4 内存读取实现，纯只读
- 提供完整的 IL2CPP 动态布局解析框架
- 类名混淆时用「语义指纹」定位字段（静态自引用 Singleton + `List<Rig>` + 字段名模糊匹配）
- 依赖 Il2CppDumper 的 `script.json` + `dump.cs`

**核心文件**

```
rf4_il2cpp_dynamic_reader.py    29 KB   IL2CPP 动态读取器
rf4_direct_source.py           117 KB   直接内存数据源
test_il2cpp_dynamic_reader.py    6 KB   回归测试
```

**已实现**：咬钩提醒、鱼名、重量、三竿槽位映射

**本地文件**：`refs/RF4-DSen_il2cpp_reader.py`（不入库）

---

### 2.2 RussianFishing4Reverse（含托管程序集）

```
https://github.com/luyikk/RussianFishing4Reverse
C# / 11★ / 2020-01
```

含 `Assembly-CSharp.dll`（10.4 MB，2020 年版本）。

现在游戏已是 IL2CPP，**该 DLL 的字段偏移不可用**，但类结构和命名仍有参考价值。

```powershell
# 下载
curl -L -o Assembly-CSharp.dll https://raw.githubusercontent.com/luyikk/RussianFishing4Reverse/master/Assembly-CSharp.dll
# 反编译
ILSpy / dnSpy
```

---

## 3. 屏幕自动化参考（不读内存）

| 仓库 | 语言 | 星 | 说明 |
|---|---|---|---|
| [dereklee0310/RussianFishing4Script](https://github.com/dereklee0310/RussianFishing4Script) | Python | 153 | 最成熟，路亚/水底/海钓/浮漂四模式，自动摩擦刹车 |
| [Binshifter/RussianFishing4Script](https://github.com/Binshifter/RussianFishing4Script) | Python | 2 | 中文，AI 判鱼、重登卖鱼、自动装配 |
| [smluntan/RussianFishing4-Tool](https://github.com/smluntan/RussianFishing4-Tool) | Python | 9 | 搬砖脚本，含点位截图库 |
| [CoderMzx/RussianFishing4-Script](https://github.com/CoderMzx/RussianFishing4-Script) | Python | 7 | 中文全自动 |
| [lzsforge/RussianFishing4-Script](https://github.com/lzsforge/RussianFishing4-Script) | Python | 5 | 全自动 |
| [Champ10nRR4/RussianFishing4Bot](https://github.com/Champ10nRR4/RussianFishing4Bot) | C# | 0 | 钓鱼 + 制作饮料 |
| [e3e0261f/RF4BOT](https://github.com/e3e0261f/RF4BOT) | AHK | 1 | AutoHotkey |
| [nelosss/Russian-fishing-4-data-syphon](https://github.com/nelosss/Russian-fishing-4-data-syphon) | Python | 1 | 数据采集与上鱼日志 |

---

## 4. 数据 / 工具类

| 仓库 | 说明 |
|---|---|
| [KoleRF4/castmark](https://github.com/KoleRF4/castmark) | 地图与钓点追踪 |
| [Nalathan01/rf4-companion](https://github.com/Nalathan01/rf4-companion) | 目标鱼/钓点/饵料/周记录 |
| [hurfy/rf4-api](https://github.com/hurfy/rf4-api) | 非官方渔获 API |
| [FliXis24/Russian-Fishing-4-Offline-Maps](https://github.com/FliXis24/Russian-Fishing-4-Offline-Maps) | 离线地图 |
| [Binshifter/RussianFishing4-ChineseLanguagePatch](https://github.com/Binshifter/RussianFishing4-ChineseLanguagePatch) | 客户端汉化 |

---

## 5. ⚠️ 垃圾仓库名单（不要参考）

2026-11-22 前后集中出现一批 **SEO 垃圾仓库**，描述华丽但**零代码**
（`language: null`，文件仅几 KB 全是 README 堆关键词）：

```
Hunternepinpoint/rf4-bait-lab
riverscorpiontrellis/rf4-tackle-builder
MorningKnight/rf4-trophy-hunter          ← 描述写 "Trophy Fish ESP"
mrhi21/get-russian-fishing-4-chicory     ← 描述写 "HWID spoofer, aimbot and ESP"
Nalathan01/rf4-companion                 ← 有空壳嫌疑，用前先看文件数
```

**判定依据**：创建时间集中在同一天、无语言标记、star 数异常整齐（7~9）、
文件数少且全是 txt/md。

---

## 6. 调研渠道备忘

| 渠道 | 状态 | 说明 |
|---|---|---|
| **unknowncheats.me** | ⚠️ Cloudflare 保护 | `search.php` 返回 403 `challenge_required`，无法程序化抓取，需手动登录检索 |
| **GitHub API** | ✅ 可用 | 搜索仓库无需认证 |
| **grep.app**（代码搜索） | ⚠️ 限流 | 返回 429 |
| Bing / DuckDuckGo | ❌ 无效 | 中文站忽略查询词；ddg 直连失败 |

**UC 建议搜索词**：`Russian Fishing 4`、`RF4`、`rf4_x64`、`RF4-DSen`
