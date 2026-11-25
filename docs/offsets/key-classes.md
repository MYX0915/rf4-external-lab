# 核心类字段表

> 来源：`sdk/rf4_x64_20261108_200736/dump.cs`（UnityDumper 导出，共 425575 行）
> 提取脚本：`tools/extract_classes.py`、`tools/extract_more.py`
> 完整定义：`refs/rf4_key_classes.txt`（本地，不入库）

---

## ⚠️ 字段名说明

所有游戏类的字段名被 **Beebyte 混淆**（如 `ellmppcbkao`、`oljcomcodha`），
**类型和偏移是准确的**，但字段语义需要通过游戏内实测标定（快照对比法）。

例外：少量字段保留了原名，这些是**高价值锚点**：

```
MyBoid.minX / maxX / minZ / maxZ / minY / maxY
MyBoid.visibleByCamera / velocity
MyBoid.tr / species / state
Rig.WarningRodTestFactor / CriticalRodTestFactor
RodRestController.externalByteAlarm / deleteAfterDisconnect
```

---

## 1. ⭐ `RF4.Client.MyBoid` —— 游动鱼实体

社区所说的「水下轻量鱼 ESP」和「鱼群点位标记」就在这里。

```csharp
public static List<RF4.Client.MyBoid> adaoecddedc;   // static +0x0   全部游动鱼
public static List<RF4.Client.BeaconGizmo> bocpjpmlkll; // static +0x8  信标

protected System.Single aehfadmfnji;   // 0x20
protected System.Single nmloaghfngf;   // 0x24
protected System.Single jpaleliaihk;   // 0x28
protected System.Single pfifoedkdld;   // 0x2C

public System.Single minX;   // 0x30   ┐
public System.Single maxX;   // 0x34   │
public System.Single minZ;   // 0x38   │ 活动范围
public System.Single maxZ;   // 0x3C   │
public System.Single minY;   // 0x40   │
public System.Single maxY;   // 0x44   ┘

protected System.Single lhmjcehhief;  // 0x48
protected System.Single inaeaiaebcm;  // 0x4C
protected System.Single ockcmkfkdia;  // 0x50
protected System.Single paedjbimplk;  // 0x54

public System.Boolean visibleByCamera; // 0x58   是否在相机视野内
public System.Single minFlockingTime;  // 0x5C
public System.Single maxFlockingTime;  // 0x60
public System.Single minRestingTime;   // 0x64
public System.Single maxRestingTime;   // 0x68

protected UnityEngine.Animator dbkgphpoajp;  // 0x70
private UnityEngine.Vector3 amdldkbppia;     // 0x78
protected UnityEngine.Vector3 jamcobfgbei;   // 0x84
protected UnityEngine.Vector3 bioejnpeahk;   // 0x90
protected UnityEngine.Quaternion aaaafokihad; // 0x9C
protected UnityEngine.Quaternion molbldgmhka; // 0xAC

public UnityEngine.Transform tr;         // 0xC0   ← 世界坐标入口
public UnityEngine.Vector3 velocity;      // 0xC8   ← 游动速度
public ilflkbbmbmp species;              // 0xD4   ← 鱼种类别
public fabgpofmbif state;                // 0xD8   ← 状态
private readonly System.Int32 kakpmodlgkp; // 0xDC
protected RF4.Client.BeaconGizmo llhgpjdpdin; // 0xE0
public System.Single _maxSteerForce;     // 0xE8
public System.Single _maxSpeed;          // 0xEC
public System.Single _minSpeed;          // 0xF0
```

### 相关枚举

```csharp
public enum ilflkbbmbmp : int {   // species
    gmemihdahnc = 0,
    jcfhafifkin = 1,
    ihkppididdg = 2,
}

public enum fabgpofmbif : int {   // state
    ocmdhcbbagm = 0,
    monleojgadg = 1,
    mpgiocnmbll = 2,
    banbgcmbihj = 3,
    indigidgndi = 4,
}
```

> 枚举值含义需在游戏内实测确认（对照画面里鱼的状态变化）。

### 取坐标

```
MyBoid 是 MonoBehaviour 派生（含 Transform tr 字段）
→ tr + 0x38 附近是该 Transform 的 m_LocalPosition
更稳的做法：用 UnityEngine.Transform.get_position_Injected，
   传 tr 的 m_CachedPtr（tr + 0x10）
```

---

## 2. ⭐ `RF4.Client.FishingScene.RodRestController` —— 竿位

U1/U2/U3 槽位映射靠它（社区用法：三根竿的坐标共识，不用地址顺序）。

```csharp
internal class RodRestController : UnityEngine.MonoBehaviour
{
    protected oljcomcodha manndckkhde;                            // 0x20
    private UnityEngine.Quaternion <gagomngadgc>k__BackingField; // 0x28  ← 朝向
    private System.Action<RodRestController> gombcndkpgo;         // 0x38
    private UnityEngine.Vector3 <lpgfhbclfck>k__BackingField;    // 0x40  ← 世界坐标
    public System.Boolean externalByteAlarm;                      // 0x4C
    private System.Boolean <ihglalokcil>k__BackingField;          // 0x4D
    protected UnityEngine.Transform cicmhgacljm;                  // 0x50
    private phmgbolhlai <mcinepgnien>k__BackingField;             // 0x58
    public System.Boolean deleteAfterDisconnect;                  // 0x60
}
```

---

## 3. `RF4.Client.FishingScene.Rig` —— 单根竿（拉力/磨损）

```csharp
internal abstract class Rig : RF4.Client.GameBehaviour, ...

public const System.Single WarningRodTestFactor  = 1.02499998f;  // 预警阈值
public const System.Single CriticalRodTestFactor = 1.10000002f;  // 危险阈值

private System.Action<dcklidpgglo> colbmcbfdmj;  // 0x30
private System.Action<dcklidpgglo> jjkoimiccgk;  // 0x38
private UnityEngine.Transform ofanjomopko;       // 0x40
private System.Double <fhgbchhopia>k__BackingField; // 0x48
protected ebabccgcfjl lmkifikkfkn;               // 0x50
private System.Single jeolelhmflo;               // 0x58
protected oljcomcodha maodcphblpk;               // 0x60
private System.Boolean meghhnalieg;              // 0x68
protected UnityEngine.Vector2 kjkfofnkeml;       // 0x6C
private System.Single ddcmomlapma;               // 0x74
private readonly jgamblffchk gbedcpnnfpo;        // 0x78
private System.Single alkbhicjjnn;               // 0x80
private System.Double <dhdlbiaebjo>k__BackingField; // 0x88
protected effjdeblglh coffdfkgfaa;               // 0x90
private UnityEngine.Transform jedmopeicbe;       // 0x98
private System.Action<Rig> pfkppojdlip;          // 0xA0
private System.Single <edikjnhjbmb>k__BackingField; // 0xA8
private dcklidpgglo.kifmdggkfdb <ohompkhhijl>k__BackingField; // 0xAC
private System.Boolean <dlblgjodfha>k__BackingField; // 0xB0
private System.Action<Rig> ibeaelebnml;          // 0xB8
```

**待标定**：30+ 个 `Single`/`Double` 字段中，哪些是
拉力 / 最大安全拉力 / 线磨损 / 鱼重 / 搏鱼进度。

---

## 4. `RF4.Client.GameController` —— 主入口单例

```csharp
private static RF4.Client.GameController ellmppcbkao;   // static 0x0  ← 单例
private static readonly iaikbaoignh dpakaefemlc;        // static 0x8
protected RF4.Client.UIController m_ui;                 // 0x20
protected RF4.Client.Sound.GameSoundController m_sound; // 0x28
private nnfmkefpbdi.nfngambecpd ihgcehcapci;            // 0x30
protected RF4.Client.GameLevelLoader m_levelLoader;     // 0x38
```

---

## 5. `RF4.Client.FishingScene.FishingSet` —— 当前渔具组合

```csharp
internal class RF4.Client.FishingScene.FishingSet
// 389 行字段，完整定义见本地 refs/rf4_key_classes.txt
```

继承链：

```
FishingSet
 └── RodRigFishingSet
       └── RodReelRigFishingSet
```

从输入控制器可以直接拿到当前 FishingSet：

```csharp
internal class FishingSetUserInputController
{
    public RF4.Client.FishingScene.FishingSet set;  // 0x40
}
```

---

## 6. `RF4.Client.FishingScene.Fisher` —— 玩家对象

```
327 行字段（坐标、等级、银币、体力、饥饿等，均为混淆名）
完整定义见本地 refs/rf4_key_classes.txt
```

RF4-DSen 的做法：用 Fisher 只做只读集合发现，定位当前 live `FishingSet`。

---

## 7. `RF4.Client.Assets.FishAssetCollection` —— 鱼类资产表

```csharp
internal class FishAssetCollection
    static +0x0     ehjeienekge            ← 单例
    +0x20           FishFamilyAsset[] assets
```

配合 `StringLiterals/StringLiterals.txt` 可把 `species` 枚举和鱼名对上。

---

## 8. `RF4.Client.FishingScene.FishingZonesManager`

```csharp
static +0x0    <kanghfjgcak>k__BackingField    ← 单例
```

钓区管理，可能提供当前所在钓区/可用鱼种。

---

## 9. RF4.Client.* 单例速查表

| 类 | 静态偏移 | 实例字段 | 用途 |
|---|---|---|---|
| `RF4.Client.GameController` | `+0x0` | `ellmppcbkao` | 主入口 |
| `RF4.Client.FishingScene.FishingZonesManager` | `+0x0` | `<kanghfjgcak>` | 钓区 |
| `RF4.Client.Assets.FishAssetCollection` | `+0x0` | `ehjeienekge` | 鱼类资产 |
| `RF4.Client.Weather.WeatherManager` | `+0x38` | `ellmppcbkao` | 天气/时间/水温 |
| `RF4.Client.WaterTextures` | `+0x20` | `jagiidlgefn` | 水面 |
| `RF4.Client.AllLOD` | `+0x0` | `ellmppcbkao` | LOD |
| `RF4.Client.School.SchoolController` | `+0x0` | `<kanghfjgcak>` | 教学 |
| `RF4.Client.MyBoid` | `+0x0` | `adaoecddedc` : `List<MyBoid>` | 游动鱼 |
| `RF4.Client.MyBoid` | `+0x8` | `bocpjpmlkll` : `List<BeaconGizmo>` | 信标 |
| `RF4.Client.FishingScene.CampFireWatchDog` | `+0x0` | `ijbogjfoall` | 篝火 |
| `RF4.Client.UI.RUIRoot` | `+0x0` | `ellmppcbkao` | UI 根 |

> `static +N` 表示该偏移是相对 `Il2CppClass.static_fields` 的。
> 读法：`static_fields = read_u64(class + 0xB8)`，再 `read_u64(static_fields + N)`。

---

## 10. 待标定清单

| 目标 | 所在类 | 方法 |
|---|---|---|
| 拉力 / 最大安全拉力 | `Rig` | 拉鱼时快照对比 |
| 线磨损 | `Rig` 或引线对象 | 反复抛投后对比 |
| 鱼重 / 鱼种 | 咬钩战斗鱼（**非 MyBoid**） | 上鱼瞬间快照 |
| 体力 / 饥饿 / 疲劳 | `Fisher` | 进食前后对比 |
| 银币 / 经验 / 等级 | `Fisher` 或玩家存档对象 | 卖鱼前后对比 |
| 背包物品 | Inventory 容器（待定位） | 增减物品对比 |
| 水温水压 | `WeatherManager` | 对照 HUD 显示 |


---

## 11. TypeInfo 槽地址（RVA，相对 GameAssembly.dll）

> 来源：`Il2CppHeader/script.json` 的 `ScriptMetadata`（11254 条）
> 交叉验证：`PointerPaths.txt` 中的同三条链完全一致 ✓

| 类 | TypeInfo RVA | PointerPaths 对照 |
|---|---|---|
| **`RF4.Client.MyBoid`** | **`0x040C7FE8`** | 不在链表中（静态未初始化） |
| `RF4.Client.GameController` | `0x040D8F88` | ✅ `["GameAssembly.dll"+40D8F88]+B8]+0` |
| `RF4.Client.FishingScene.FishingZonesManager` | `0x040A09D0` | ✅ `["GameAssembly.dll"+40A09D0]+B8]+0` |
| `RF4.Client.Weather.WeatherManager` | `0x040C8330` | ✅ `["GameAssembly.dll"+40C8330]+B8]+38` |
| `RF4.Client.FishingScene.Fisher` | `0x040C2408` | — |
| `RF4.Client.Assets.FishFamilyAsset` | `0x040C97C8` | — |

读取链：

```python
classPtr     = read_u64(module_base + RVA)      # Il2CppClass*
staticFields = read_u64(classPtr + 0xB8)        # void*
instance     = read_u64(staticFields + 字段偏移)
```

**注意**：槽存在 ≠ 已初始化。类没被触碰过时槽里是 0。
`MyBoid` 的槽在进钓点后才会填上。

---

## 12. Rig 的具体子类（metadata 里已注册的）

抽象类 `Rig` 本身没有独立 TypeInfo 槽，实际存在的是这些子类：

```
RF4_Client_FishingScene_RigMarinePilker
RF4_Client_FishingScene_RigLureComplex
RF4_Client_FishingScene_RigBobberCarp
RF4_Client_FishingScene_RigBobberClassic
```

还有 `RF4_Client_FishingScene_Rod`、`RF4_Client_Assets_RodAsset`、`RF4_Client_Assets_RodPodAsset`。

`FishingSet` 本身也没有独立槽（只有它的编译器生成嵌套类型有）。

---

## 13. Transform 位置读取的难点

`UnityEngine.Transform` 在 dump.cs 里**只有属性没有字段**：

```csharp
public class UnityEngine.Transform : UnityEngine.Component
{
    // Properties
    public UnityEngine.Vector3 position { get; set; }        ← 无 backing field
    public UnityEngine.Vector3 localPosition { get; set; }   ← 无 backing field
    ...
}
```

数据在原生侧（`m_CachedPtr` at +0x10 指向原生 Transform），
原生结构布局随 Unity 版本变化，不能硬读。

**规避方案**：`MyBoid` 自己带了 3 个 Vector3 候选字段：

```
+0x78  amdldkbppia
+0x84  jamcobfgbei
+0x90  bioejnpeahk
```

其中一个就是世界坐标。用 `src/reader/probe_myboid.py` 在游戏里跑一遍，
看哪个落在合理的地图坐标范围内即可确认。
