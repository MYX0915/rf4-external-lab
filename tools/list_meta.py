import io, json
d = json.loads(io.open(r"sdk\rf4_x64_20261108_200736\Il2CppHeader\script.json", encoding="utf-8", errors="replace").read())
names = [e["Name"][:-len("_TypeInfo")] for e in d["ScriptMetadata"] if e.get("Name","").endswith("_TypeInfo")]
print("TypeInfo 总数:", len(names))
for kw in ("FishingSet","RodRest","_Rig","Rig_","Rod" ,"FishAsset","Beacon"):
    hits = [n for n in names if kw in n]
    print("\n--- 含 '%s' 的: %d 个 ---" % (kw, len(hits)))
    for h in hits[:12]:
        print("   ", h)
