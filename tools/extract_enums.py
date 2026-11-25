import io, re
DUMP = r"sdk\rf4_x64_20261108_200736\dump.cs"
src = io.open(DUMP, encoding="utf-8", errors="replace").read()
lines = src.split("\n")
pat = re.compile(r"^(?:public |internal |private |protected )?(?:sealed |abstract |static )*(?:class|struct|enum|interface)\s+([\w\.`<>,\[\]\+]+)")
idx = {}
for i, ln in enumerate(lines):
    m = pat.match(ln)
    if m: idx.setdefault(m.group(1), i)
for t in ["ilflkbbmbmp", "fabgpofmbif"]:
    if t not in idx:
        # 可能是全局命名空间
        found = [k for k in idx if k.endswith("." + t)]
        print("未找到 %s，相似: %s" % (t, found[:5])); continue
    s = idx[t]; depth=0; started=False; end=s
    for j in range(s, min(s+3000, len(lines))):
        depth += lines[j].count("{") - lines[j].count("}")
        if "{" in lines[j]: started=True
        if started and depth<=0: end=j; break
    body = lines[s:end+1]
    print("\n" + "="*60)
    print("// %s  (共 %d 行)" % (t, len(body)))
    for ln in body[:45]: print(ln.rstrip())
