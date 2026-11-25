"""probe_myboid.py —— 验证 MyBoid（游动鱼）链表并找出真正的世界坐标字段

游戏必须已在钓点里（有鱼游动）。

用法:
    python probe_myboid.py            # 列出前 10 条鱼的全部候选字段
    python probe_myboid.py -n 30      # 看前 30 条
"""
import argparse
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rf4_mem import RF4, IL2CPP_CLASS_NAME  # noqa: E402

# TypeInfo 槽 RVA（来自 script.json ScriptMetadata）
TYPEINFO = {
    "MyBoid": 0x040C7FE8,
    "GameController": 0x040D8F88,
    "Fisher": 0x040C2408,
    "FishingZonesManager": 0x040A09D0,
    "FishFamilyAsset": 0x040C97C8,
    "WeatherManager": 0x040C8330,
}

# MyBoid 静态字段
MYBOID_STATIC_LIST = 0x0        # List<MyBoid> adaoecddedc
MYBOID_STATIC_BEACON = 0x8      # List<BeaconGizmo> bocpjpmlkll

# MyBoid 实例字段（候选坐标）
CANDIDATE_VEC3 = [
    (0x78, "amdldkbppia"),
    (0x84, "jamcobfgbei"),
    (0x90, "bioejnpeahk"),
    (0xA0, "(0x9C Quaternion 之后)"),
]
OTHER_FIELDS = [
    (0x58, "bool", "visibleByCamera"),
    (0x5C, "f32", "minFlockingTime"),
    (0x60, "f32", "maxFlockingTime"),
    (0x64, "f32", "minRestingTime"),
    (0x68, "f32", "maxRestingTime"),
    (0xC8, "vec3", "velocity"),
    (0xD4, "i32", "species"),
    (0xD8, "i32", "state"),
    (0xDC, "i32", "kakpmodlgkp"),
    (0xE8, "f32", "_maxSteerForce"),
    (0xEC, "f32", "_maxSpeed"),
    (0xF0, "f32", "_minSpeed"),
]
BOUNDS = [(0x30, "minX"), (0x34, "maxX"), (0x38, "minZ"),
          (0x3C, "maxZ"), (0x40, "minY"), (0x44, "maxY")]


def plausible(v):
    """粗略判断像不像一张地图上的坐标"""
    x, y, z = v
    if any(math.isnan(a) or math.isinf(a) for a in v):
        return False
    if abs(x) > 20000 or abs(y) > 20000 or abs(z) > 5000:
        return False
    if abs(x) < 1 and abs(z) < 1:
        return False
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-n", type=int, default=10)
    args = ap.parse_args()

    g = RF4()
    print("进程 PID      :", g.pid)
    print("GameAssembly  : 0x%X  (大小 0x%X)" % (g.mod_base, g.mod_size))

    # ---- 先验证已知能用的类 ----
    print("\n=== TypeInfo 自检 ===")
    for name, rva in TYPEINFO.items():
        cls = g.il2cpp_class(rva)
        nm = g.class_name(cls) if cls else ""
        print("  %-22s RVA 0x%08X → class 0x%-12X name=%s" % (name, rva, cls, nm or "?"))
        if cls and not nm:
            print("       ⚠ class 读到了但名字为空，可能 RVA 不对或类未初始化")

    # ---- MyBoid 链表 ----
    print("\n=== MyBoid 游动鱼链表 ===")
    lst = g.static_ptr(TYPEINFO["MyBoid"], MYBOID_STATIC_LIST)
    print("  static List 指针: 0x%X" % lst)
    if not lst:
        print("  ✗ 为空 —— 可能：")
        print("    · 不在钓点（需要进图才有鱼）")
        print("    · 版本不匹配（dump 是 2026-11-08 的）")
        print("    · TypeInfo 槽未初始化")
        return
    items = g.list_items(lst)
    print("  鱼数量: %d" % len(items))

    beacon = g.static_ptr(TYPEINFO["MyBoid"], MYBOID_STATIC_BEACON)
    if beacon:
        print("  BeaconGizmo 数量: %d" % len(g.list_items(beacon)))

    if not items:
        return

    # ---- 逐条 dump ----
    print("\n=== 前 %d 条明细 ===" % min(args.n, len(items)))
    for i, obj in enumerate(items[:args.n]):
        print("\n  [%d] 实例 0x%X" % (i, obj))
        print("      --- 候选坐标 ---")
        for off, nm in CANDIDATE_VEC3:
            v = g.vec3(obj + off)
            flag = "✓" if plausible(v) else " "
            print("      %s +0x%02X %-22s (%.2f, %.2f, %.2f)" % (flag, off, nm, v[0], v[1], v[2]))
        print("      --- 其他字段 ---")
        for off, typ, nm in OTHER_FIELDS:
            if typ == "f32":
                print("        +0x%02X %-18s %.3f" % (off, nm, g.f32(obj + off)))
            elif typ == "i32":
                print("        +0x%02X %-18s %d" % (off, nm, g.i32(obj + off)))
            elif typ == "vec3":
                v = g.vec3(obj + off)
                print("        +0x%02X %-18s (%.2f, %.2f, %.2f)" % (off, nm, v[0], v[1], v[2]))
            elif typ == "bool":
                print("        +0x%02X %-18s %s" % (off, nm, bool(g.read(obj + off, 1)[0])
                                                     if g.read(obj + off, 1) else "?"))
        print("      --- 活动范围 ---")
        b = {nm: g.f32(obj + off) for off, nm in BOUNDS}
        print("        X[%.0f ~ %.0f]  Z[%.0f ~ %.0f]  Y[%.0f ~ %.0f]"
              % (b["minX"], b["maxX"], b["minZ"], b["maxZ"], b["minY"], b["maxY"]))

    g.close()


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as e:
        print("\n[错误] %s" % e)
        print("\n检查清单：")
        print("  1. 游戏是否在运行（进程名 rf4_x64.exe）")
        print("  2. 是否以管理员权限运行本脚本")
        print("  3. 是否已进入钓点（主菜单没有鱼对象）")
        sys.exit(1)
