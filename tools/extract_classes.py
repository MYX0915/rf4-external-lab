"""从 dump.cs 提取指定类的完整定义块"""
import io
import re
import sys

DUMP = r"E:\Users\MYX\Desktop\Russian Fishing 4\sdk\rf4_x64_20261108_200736\dump.cs"
OUT = r"E:\Users\MYX\Desktop\Russian Fishing 4\refs\rf4_key_classes.txt"

TARGETS = [
    "RF4.Client.FishingScene.FishingSet",
    "RF4.Client.FishingScene.RodRigFishingSet",
    "RF4.Client.FishingScene.RodReelRigFishingSet",
    "RF4.Client.FishingScene.RodRestController",
    "RF4.Client.FishingScene.Fisher",
    "RF4.Client.FishingScene.RodRig",
    "RF4.Client.FishingScene.Rig",
    "RF4.Client.FishingScene.FishingZonesManager",
    "RF4.Client.Assets.FishAssetCollection",
    "RF4.Client.Assets.FishFamilyAsset",
]


def main():
    src = io.open(DUMP, encoding="utf-8", errors="replace").read()
    lines = src.split("\n")
    print("dump.cs 总行数: %d" % len(lines))

    # 建立 类型名 -> 起始行 的索引
    idx = {}
    pat = re.compile(r"^(?:public |internal |private |protected )?(?:sealed |abstract |static )*"
                     r"(?:class|struct|enum|interface)\s+([\w\.`<>,\[\]\+]+)")
    for i, ln in enumerate(lines):
        m = pat.match(ln)
        if m:
            idx.setdefault(m.group(1), i)

    out = []
    for t in TARGETS:
        if t not in idx:
            out.append("\n" + "=" * 70 + "\n[未找到] %s\n" % t)
            continue
        s = idx[t]
        # 从起始行往后找配对的大括号
        depth = 0
        started = False
        end = s
        for j in range(s, min(s + 4000, len(lines))):
            depth += lines[j].count("{") - lines[j].count("}")
            if "{" in lines[j]:
                started = True
            if started and depth <= 0:
                end = j
                break
        block = lines[s:end + 1]
        out.append("\n" + "=" * 70 + "\n// %s   (dump.cs 行 %d ~ %d, 共 %d 行)\n" % (t, s + 1, end + 1, len(block)))
        out.extend(block)
        print("提取 %-50s %4d 行" % (t, len(block)))

    io.open(OUT, "w", encoding="utf-8").write("\n".join(out))
    print("\n已写入:", OUT)


if __name__ == "__main__":
    main()
