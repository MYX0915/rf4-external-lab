import io, re
DUMP = r"sdk\rf4_x64_20261108_200736\dump.cs"
lines = io.open(DUMP, encoding="utf-8", errors="replace").read().split("\n")
pat = re.compile(r"^(?:public |internal |private |protected )?(?:sealed |abstract |static )*(?:class|struct)\s+([\w\.`<>,\[\]\+]+)")
idx = {}
for i, ln in enumerate(lines):
    m = pat.match(ln)
    if m: idx.setdefault(m.group(1), i)
for t in ["UnityEngine.Transform", "UnityEngine.Component", "UnityEngine.Object"]:
    if t not in idx: print("未找到", t); continue
    s = idx[t]; depth=0; started=False; end=s
    for j in range(s, min(s+2000, len(lines))):
        depth += lines[j].count("{") - lines[j].count("}")
        if "{" in lines[j]: started=True
        if started and depth<=0: end=j; break
    print("\n" + "="*60)
    print("// %s  (共 %d 行)" % (t, end-s+1))
    for ln in lines[s:min(s+25,end+1)]:
        print(ln.rstrip())
