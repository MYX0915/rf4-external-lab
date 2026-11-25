# driver/ —— 备用方案（当前未使用）

本目录的内核驱动是**备用方案**，当前不参与构建、不加载。

---

## 当前选型：用户态读取

RF4 用 **用户态 `OpenProcess` + `ReadProcessMemory`**，理由见
[`docs/method-choice.md`](../docs/method-choice.md)。

```
src/reader/rf4_mem.py       用户态读取库
src/reader/probe_myboid.py  验证工具
```

---

## 这个驱动什么时候才用

只要出现下面任意一条，才切到驱动：

```
1. ReadProcessMemory 返回 0 / 数据明显不对（被拦截）
2. OpenProcess 拿不到句柄
3. 进程列表里出现可疑的内核保护模块
4. 读取触发崩溃或被踢
```

---

## 驱动做了什么（如果要用）

`rf4drv.c` —— kdmapper 兼容，只读为主：

**职责**：在内核里完成 IL2CPP 链路解析，把结果写成紧凑二进制 feed，
用户态只读 feed，避免每次读取都走文件 IPC。

**链路**

```
module_base + TypeInfo RVA
  → Il2CppClass*
    → +0xB8 static_fields
      → +静态偏移 List<MyBoid>
        → +0x10 items / +0x18 size
          → items + 0x20 + i*8 → MyBoid*
            → +坐标偏移 Vector3
```

**文件 IPC**

```
C:\ProgramData\rf4log.txt     日志（追加）
C:\ProgramData\rf4_req.txt    请求（读后自删）
C:\ProgramData\rf4_ack.txt    应答
C:\ProgramData\rf4_feed.bin   连续发布的实体数据（60fps）
```

**命令**

| 命令 | 作用 |
|---|---|
| `stat` | 状态（模块基址、类指针、List 指针、鱼数量） |
| `m` | GameAssembly.dll 基址 |
| `r <va> <size>` | 读原始字节（hex） |
| `q <va> [n]` | 读 n 个 u64 |
| `pos <off>` | **运行时**切换 MyBoid 坐标偏移（不用重编译） |
| `feed 0\|1` | 开关连续发布 |
| `probe [n]` | dump 前 n 条 boid 全部候选字段（坐标标定用） |
| `w <va> <hex>` | 写字节（慎用） |

`pos` 命令是关键：坐标标定要反复试 `0x78 / 0x84 / 0x90`，
如果每次都要重编译 + 重启，根本没法迭代。运行时切偏移才可行。

---

## 状态

```
[ ] 未编译验证（需要 WDK，且当前不需要）
[ ] 未在 RF4 上测试过
```

如果能用用户态，这个文件会一直闲置。保留它是为了万一。
