"""从 script.json 的 ScriptMetadata 提取 TypeInfo 槽地址（RVA）"""
import io
import json

SCRIPT = r"E:\Users\MYX\Desktop\Russian Fishing 4\sdk\rf4_x64_20261108_200736\Il2CppHeader\script.json"
OUT = r"E:\Users\MYX\Desktop\Russian Fishing 4\refs\typeinfo_addresses.txt"

TARGETS = [
    "RF4.Client.MyBoid",
    "RF4.Client.GameController",
    "RF4.Client.FishingScene.FishingSet",
    "RF4.Client.FishingScene.RodRigFishingSet",
    "RF4.Client.FishingScene.RodReelRigFishingSet",
    "RF4.Client.FishingScene.RodRestController",
    "RF4.Client.FishingScene.Rig",
    "RF4.Client.FishingScene.Fisher",
    "RF4.Client.FishingScene.FishingZonesManager",
    "RF4.Client.Assets.FishAssetCollection",
    "RF4.Client.Assets.FishFamilyAsset",
    "RF4.Client.Weather.WeatherManager",
    "RF4.Client.BeaconGizmo",
    "RF4.Client.FishingScene.FishingSetUserInputController",
]


def main():
    d = json.loads(io.open(SCRIPT, encoding="utf-8", errors="replace").read())
    meta = d["ScriptMetadata"]

    # Name 形如 RF4_Client_MyBoid_TypeInfo
    idx = {}
    for e in meta:
        n = e.get("Name", "")
        if n.endswith("_TypeInfo"):
            key = n[:-len("_TypeInfo")]
            idx.setdefault(key, e["Address"])

    lines = []
    print("=== TypeInfo 槽地址（RVA，相对 GameAssembly.dll）===")
    for t in TARGETS:
        key = t.replace(".", "_")
        addr = idx.get(key)
        if addr is None:
            # 有些类型名带 `/` `<` `>` 等，做一次宽松匹配
            cand = [k for k in idx if k.endswith("_MyBoid")] if "MyBoid" in t else []
            if cand:
                addr = idx[cand[0]]
                key = cand[0]
        if addr is not None:
            s = "%-56s 0x%08X   (十进制 %d)" % (t, addr, addr)
            print("  " + s)
            lines.append("%-56s 0x%08X" % (t, addr))
        else:
            print("  %-56s 未找到" % t)

    io.open(OUT, "w", encoding="utf-8").write(
        "// TypeInfo 槽地址（RVA，相对 GameAssembly.dll 基址）\n"
        "// 来源: Il2CppHeader/script.json 的 ScriptMetadata\n"
        "//\n"
        "// 读取链:\n"
        "//   classPtr      = read_u64(module_base + RVA)      // Il2CppClass*\n"
        "//   staticFields  = read_u64(classPtr + 0xB8)        // void*\n"
        "//   instance      = read_u64(staticFields + 字段偏移)\n"
        "//\n"
        "// 注意: 槽存在 ≠ 已初始化；类没被触碰过时该槽为 0\n"
        "//       需要先在游戏里触发对应流程（进钓点 / 打开背包等）\n\n"
        + "\n".join(lines) + "\n")
    print("\n已写入:", OUT)


if __name__ == "__main__":
    main()
