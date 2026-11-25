# IL2CPP 运行时常量表

> 来源：`sdk/rf4_x64_20261108_200736/` 的 `Health.txt`（UnityDumper 实测）
> 交叉验证：`refs/RF4-DSen_il2cpp_reader.py`（RF4-DSen V1.08 常量）

---

## 1. 环境

| 项目 | 值 |
|---|---|
| 游戏 | `rf4_x64` |
| Runtime | IL2CPP |
| 主模块 | `GameAssembly.dll` |
| 模块大小 | `0x4855000`（约 75.8 MB） |
| Unity 版本 | `2022.3.62f3` (96770f904ca7) |
| 指针宽度 | 8 字节 |
| 对象头 | `0x10` |
| dump 时模块基址 | `0x7FFEEBC50000`（每次启动变化，不可硬编码） |

---

## 2. Il2CppClass 布局

| 偏移 | 类型 | 含义 | 来源 |
|---|---|---|---|
| `+0x10` | `char*` | 类型名 | RF4-DSen 常量 + Health PASS |
| `+0x18` | `char*` | 命名空间 | RF4-DSen 常量 |
| `+0xB8` | `void*` | `static_fields` | **Health.txt 实测确认** |

`StaticData` 访问方式：

```python
static_fields = read_u64(il2cpp_class + 0xB8)
instance      = read_u64(static_fields + field_offset)
```

---

## 3. 容器布局

| 类型 | 偏移 | 含义 |
|---|---|---|
| `List<T>` | `+0x10` | `T[] items` |
| `List<T>` | `+0x18` | `int size` |
| `Il2CppArray` | `+0x18` | `uint length`（元素个数） |
| `Il2CppArray` | `+0x20` | 第 0 个元素 |

遍历范式：

```python
lst = read_u64(address)
items = read_u64(lst + 0x10)
size  = read_i32(lst + 0x18)
for i in range(size):
    obj = read_u64(items + 0x20 + i * 8)
```

> 注意：`List<T>` 的 `items` 指向 Il2CppArray，数组头占 0x20 字节。

---

## 4. Unity 对象布局

| 类型 | 偏移 | 含义 |
|---|---|---|
| `UnityEngine.Object` | `+0x10` | `m_CachedPtr`（原生对象指针，用于判活） |
| `System.String` | `+0x10` | `int length` |
| `System.String` | `+0x14` | `char16[] chars`（UTF-16） |
| `UnityEngine.Vector2` | — | 8 字节 |
| `UnityEngine.Vector3` | — | 12 字节 |
| `UnityEngine.Vector4` | — | 16 字节 |
| `UnityEngine.Quaternion` | — | 16 字节 |
| `UnityEngine.Color` | — | 16 字节 |
| `UnityEngine.Color32` | — | 4 字节 |
| `UnityEngine.Matrix4x4` | — | 64 字节 |
| `UnityEngine.Rect` | — | 16 字节 |
| `UnityEngine.Bounds` | — | 24 字节 |
| `UnityEngine.Ray` | — | 24 字节 |
| `UnityEngine.LayerMask` | — | 4 字节 |

以上布局均由 Health.txt 逐项核对通过：

```
[ OK ] object header is 0x10 (Vector3 boxed size 0x1C)
[ OK ] UnityEngine.Object.m_CachedPtr at 0x10
[ OK ] System.String length 0x10 / chars 0x14
[ OK ] Vector2 / Vector3 / Vector4 / Quaternion / Color / Color32
[ OK ] Matrix4x4 / Rect / Bounds / Ray / Vector2Int / Vector3Int / LayerMask
```

---

## 5. 可用入口方法（Health.txt 确认）

| 方法 | 用途 |
|---|---|
| `Object.FindObjectsOfType(Type, bool)` | 支持 includeInactive，可按类型枚举场景对象 |
| `Transform.get_position_Injected` | 快速读世界坐标（比走属性快） |
| `Camera.get_main` | 主相机 |
| `Camera` 世界矩阵 / 投影矩阵 | 纯 C++ 实现 WorldToScreen，不需要调游戏函数 |

---

## 6. 泛型示例

```
报告泛型类      58379
实际泛型实例    36583
已收集          13473
尚未初始化      23110
失败            0
```

> 泛型实例不是全部一开始就存在（懒初始化）。如果从 dump 拿到的
> `List<MyBoid>` 泛型类地址读不到内容，说明该泛型尚未实例化，
> 需要先在游戏里触发对应流程（进钓点 / 打开背包等）。

---

## 7. 字符串字面量

```
字符串字面量槽   5951
```

配合 `StringLiterals/StringLiterals.txt` 可以定位 UI 文本、鱼名、提示语，
用于把混淆后的字段和实际含义对上（例如某字段旁边就是鱼的显示名）。

---

## 8. 版本变动注意

| 项目 | 是否稳定 |
|---|---|
| `Il2CppClass` 固定头（name/namespace/static_fields） | ✅ 跨版本稳定 |
| `List<T>` / 数组布局 | ✅ 跨版本稳定 |
| Unity 基础结构体布局 | ✅ 跨版本稳定 |
| **游戏类字段偏移** | ❌ **每次更新都可能变** |
| **模块基址** | ❌ 每次启动变（用模块枚举取） |

结论：**运行时框架部分可以写死，游戏类偏移必须从 dump 读或每次重新导出。**
