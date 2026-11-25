import io, re
DUMP = r"sdk\rf4_x64_20261108_200736\dump.cs"
src = io.open(DUMP, encoding="utf-8", errors="replace").read()
lines = src.split("\n")
pat = re.compile(r"^(?:public |internal |private |protected )?(?:sealed |abstract |static )*(?:class|struct|enum|interface)\s+([\w\.`<>,\[\]\+]+)")
idx = {}
for i, ln in enumerate(lines):
    m = pat.match(ln)
    if m: idx.setdefault(m.group(1), i)
for t in ["RF4.Client.GameController", "RF4.Client.MyBoid", "RF4.Client.FishingScene.FishingSetUserInputController"]:
    if t not in idx:
        print("未找到", t); continue
    s = idx[t]; depth = 0; started = False; end = s
    for j in range(s, min(s+6000, len(lines))):
        depth += lines[j].count("{") - lines[j].count("}")
        if "{" in lines[j]: started = True
        if started and depth <= 0: end = j; break
    print("\n" + "="*66)
    print("// %s  (共 %d 行)" % (t, end-s+1))
    # 只打印字段区（Fields 到 Properties 之前）
    fields = []
    in_f = False
    for k in range(s, end+1):
        if "// Fields" in lines[k]: in_f = True; continue
        if "// Properties" in lines[k]: break
        if in_f: fields.append(lines[k].rstrip())
    print("\n".join(fields[:40]))
