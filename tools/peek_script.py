import io, json
SCRIPT = r"sdk\rf4_x64_20261108_200736\Il2CppHeader\script.json"
d = json.loads(io.open(SCRIPT, encoding="utf-8", errors="replace").read())
for k in d:
    print(k, "->", len(d[k]), "条")
    if d[k]:
        print("   样例:", json.dumps(d[k][0], ensure_ascii=False)[:300])
