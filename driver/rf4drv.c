// rf4drv.sys -- Russian Fishing 4 IL2CPP reader (kdmapper-compatible, read-only by default)
//
//  职责：在内核里完成 IL2CPP 链路解析，把结果写成紧凑二进制 feed，
//        用户态只读 feed 即可，避免每次读取都走文件 IPC。
//
//  链路：module_base + TypeInfo RVA -> Il2CppClass* -> +0xB8 static_fields
//        -> +静态偏移 List<MyBoid> -> +0x10 items / +0x18 size
//        -> items + 0x20 + i*8 -> MyBoid* -> +坐标偏移 Vector3
//
//  文件 IPC:
//    C:\ProgramData\rf4log.txt     日志（追加）
//    C:\ProgramData\rf4_req.txt    请求（读后自删）
//    C:\ProgramData\rf4_ack.txt    请求应答
//    C:\ProgramData\rf4_feed.bin   连续发布的实体数据
//
//  命令:
//    stat                状态
//    m [模块名]          模块基址
//    r <va> <size>       读原始字节（hex）
//    q <va> [count]      读 count 个 u64
//    pos <off>           设置 MyBoid 坐标字段偏移（不用重编译）
//    feed 0|1            开/关连续发布
//    probe [n]           dump 前 n 条 boid 的候选字段（坐标标定用）
//    w <va> <hex>        写字节（默认保留但慎用）

#include <ntifs.h>
#include <ntstrsafe.h>
#include <string.h>

typedef unsigned __int64 U64;
typedef unsigned int   U32;
typedef unsigned char  U8;

#define LOG_FILE  L"\\??\\C:\\ProgramData\\rf4log.txt"
#define REQ_FILE  L"\\??\\C:\\ProgramData\\rf4_req.txt"
#define ACK_FILE  L"\\??\\C:\\ProgramData\\rf4_ack.txt"
#define FEED_FILE L"\\??\\C:\\ProgramData\\rf4_feed.bin"

/* ---- IL2CPP 固定偏移（Health.txt 实测） ---- */
#define OFF_CLASS_STATIC_FIELDS  0xB8
#define OFF_LIST_ITEMS           0x10
#define OFF_LIST_SIZE            0x18
#define OFF_ARRAY_FIRST          0x20

/* ---- RF4 类型 (2026-11-08 dump) ---- */
#define RVA_TYPEINFO_MYBOID       0x040C7FE8ULL
#define OFF_MYBOID_STATIC_LIST    0x0

#define MAX_ENT 128
#define FEED_VERSION 2
#define FEED_MAGIC   0x46344652u   /* 'RF4F' little-endian */

typedef struct _RF4ENT {
    float x, y, z;
    int   species;
    int   state;
    U32   flags;
} RF4ENT;                          /* 24 字节 */

typedef struct _RF4FEED {
    U32  magic;
    U32  version;
    U32  count;
    U32  posOff;
    float player[3];
    U64  boidList;
    U64  classPtr;
    RF4ENT ent[MAX_ENT];
} RF4FEED;

static PEPROCESS g_proc = NULL;
static U64  g_modBase = 0;
static ULONG g_modSize = 0;
static U32  g_posOff = 0x78;       /* 待标定：0x78 / 0x84 / 0x90 */
static U32  g_feedOn = 1;
static U32  g_lastCount = 0;
static U64  g_lastList = 0;
static U64  g_lastClass = 0;
static FAST_MUTEX g_mtx;

#define TGT_ALIVE() (g_proc && PsGetProcessExitStatus(g_proc) == STATUS_PENDING)

NTSYSAPI NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS, PVOID, PEPROCESS, PVOID, SIZE_T, KPROCESSOR_MODE, PSIZE_T);
NTSYSAPI PVOID    NTAPI PsGetProcessPeb(PEPROCESS);
NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(ULONG, PVOID, ULONG, PULONG);

typedef struct _SPID {
    ULONG Next; ULONG Threads; LARGE_INTEGER W; ULONG HF; ULONG HW;
    ULONGLONG CT; LARGE_INTEGER Cr; LARGE_INTEGER U; LARGE_INTEGER K;
    UNICODE_STRING ImageName; LONG BP; HANDLE PID;
} SPID, *PSPID;

/* ============================ 文件与日志 ============================ */

static VOID WA(PCWSTR path, PCSTR text, ULONG len)
{
    UNICODE_STRING p; OBJECT_ATTRIBUTES oa; IO_STATUS_BLOCK io; HANDLE h;
    RtlInitUnicodeString(&p, path);
    InitializeObjectAttributes(&oa, &p, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    if (!NT_SUCCESS(ZwCreateFile(&h, FILE_APPEND_DATA | SYNCHRONIZE, &oa, &io, NULL,
                                 FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0)))
        return;
    ZwWriteFile(h, NULL, NULL, NULL, &io, (PVOID)text, len, NULL, NULL);
    ZwClose(h);
}

static VOID K(PCSTR t)
{
    size_t n = 0;
    RtlStringCchLengthA(t, 2048, &n);
    WA(LOG_FILE, t, (ULONG)n);
}

static VOID KF(PCSTR f, ...)
{
    CHAR b[400]; va_list ap;
    va_start(ap, f);
    RtlStringCchVPrintfA(b, sizeof(b), f, ap);
    va_end(ap);
    K(b);
}

static VOID WriteBufAt(PCWSTR path, PVOID buf, ULONG len)
{
    UNICODE_STRING p; OBJECT_ATTRIBUTES oa; IO_STATUS_BLOCK io; HANDLE h;
    RtlInitUnicodeString(&p, path);
    InitializeObjectAttributes(&oa, &p, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    if (!NT_SUCCESS(ZwCreateFile(&h, GENERIC_WRITE | SYNCHRONIZE, &oa, &io, NULL,
                                 FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
                                 FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0)))
        return;
    ZwWriteFile(h, NULL, NULL, NULL, &io, buf, len, NULL, NULL);
    ZwClose(h);
}

static BOOLEAN FilePresent(PCWSTR path)
{
    UNICODE_STRING p; OBJECT_ATTRIBUTES oa; IO_STATUS_BLOCK io; HANDLE h;
    RtlInitUnicodeString(&p, path);
    InitializeObjectAttributes(&oa, &p, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    if (NT_SUCCESS(ZwCreateFile(&h, GENERIC_READ | SYNCHRONIZE, &oa, &io, NULL,
                                FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0))) {
        ZwClose(h);
        return TRUE;
    }
    return FALSE;
}

/* 读请求文件后立即删除（FILE_DELETE_ON_CLOSE） */
static NTSTATUS ReadAndDropRequest(PVOID buf, ULONG cap, PULONG outLen)
{
    UNICODE_STRING p; OBJECT_ATTRIBUTES oa; IO_STATUS_BLOCK io; HANDLE h;
    NTSTATUS st;
    *outLen = 0;
    RtlInitUnicodeString(&p, REQ_FILE);
    InitializeObjectAttributes(&oa, &p, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    st = ZwCreateFile(&h, GENERIC_READ | DELETE | SYNCHRONIZE, &oa, &io, NULL,
                      FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
                      FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE, NULL, 0);
    if (!NT_SUCCESS(st)) return st;
    RtlZeroMemory(buf, cap);
    st = ZwReadFile(h, NULL, NULL, NULL, &io, buf, cap - 1, NULL, NULL);
    if (NT_SUCCESS(st) || st == STATUS_END_OF_FILE) *outLen = (ULONG)io.Information;
    ZwClose(h);
    return STATUS_SUCCESS;
}

static void SleepMs(ULONG ms)
{
    LARGE_INTEGER d;
    d.QuadPart = -((LONGLONG)ms * 10000LL);
    KeDelayExecutionThread(KernelMode, FALSE, &d);
}

/* ============================ 进程与内存 ============================ */

static ULONG FindPid(PCWSTR name, LARGE_INTEGER* oc)
{
    ULONG need = 0x20000, tries = 0, pid = 0;
    PVOID buf = NULL;
    NTSTATUS st;
    for (;;) {
        if (buf) ExFreePool(buf);
        buf = ExAllocatePool2(POOL_FLAG_NON_PAGED, need, 'pidb');
        if (!buf) return 0;
        st = ZwQuerySystemInformation(5 /*SystemProcessInformation*/, buf, need, &need);
        if (st == STATUS_INFO_LENGTH_MISMATCH) { tries++; if (tries > 8) { ExFreePool(buf); return 0; } continue; }
        if (!NT_SUCCESS(st)) { ExFreePool(buf); return 0; }
        break;
    }
    {
        PSIDP s = (PSIDP)buf;
        for (;;) {
            if (s->ImageName.Buffer && s->ImageName.Length) {
                UNICODE_STRING want;
                RtlInitUnicodeString(&want, name);
                if (RtlEqualUnicodeString(&s->ImageName, &want, TRUE)) {
                    pid = (ULONG)(ULONG_PTR)s->PID;
                    if (oc) *oc = s->CT;
                    break;
                }
            }
            if (s->Next == 0) break;
            s = (PSIDP)((PUCHAR)s + s->Next);
        }
    }
    ExFreePool(buf);
    return pid;
}

static NTSTATUS MemRead(PEPROCESS proc, U64 va, PVOID dst, SIZE_T len)
{
    SIZE_T done = 0;
    if (!va || !len) return STATUS_INVALID_PARAMETER;
    return MmCopyVirtualMemory(proc, (PVOID)(ULONG_PTR)va, PsGetCurrentProcess(), dst, len, KernelMode, &done);
}

static NTSTATUS MemWrite(PEPROCESS proc, U64 va, PVOID src, SIZE_T len)
{
    SIZE_T done = 0;
    if (!va || !len) return STATUS_INVALID_PARAMETER;
    return MmCopyVirtualMemory(PsGetCurrentProcess(), src, proc, (PVOID)(ULONG_PTR)va, len, KernelMode, &done);
}

static U64 ReadPtr(PEPROCESS proc, U64 va)
{
    U64 v = 0;
    if (NT_SUCCESS(MemRead(proc, va, &v, 8))) return v;
    return 0;
}

static int ReadI32(PEPROCESS proc, U64 va)
{
    int v = 0;
    MemRead(proc, va, &v, 4);
    return v;
}

static BOOLEAN ReadVec3(PEPROCESS proc, U64 va, float* out)
{
    return NT_SUCCESS(MemRead(proc, va, out, 12));
}

/* PEB 里按名字找模块基址 */
static U64 ModuleBase(PEPROCESS proc, PCWSTR name)
{
    U64 base = 0;
    KAPC_STATE apc;
    PPEB peb;
    KeStackAttachProcess(proc, &apc);
    peb = (PPEB)PsGetProcessPeb(proc);
    if (peb) {
        PVOID ldr = *(PVOID*)((PUCHAR)peb + 0x18);
        if (ldr) {
            LIST_ENTRY* head = (LIST_ENTRY*)((PUCHAR)ldr + 0x10);
            LIST_ENTRY* cur;
            UNICODE_STRING want;
            RtlInitUnicodeString(&want, name);
            for (cur = head->Flink; cur != head && cur; cur = cur->Flink) {
                PUCHAR e = (PUCHAR)cur;
                UNICODE_STRING bn;
                bn.Length = *(USHORT*)(e + 0x58);
                bn.MaximumLength = bn.Length;
                bn.Buffer = *(PWCH*)(e + 0x60);
                if (bn.Buffer && bn.Length && RtlEqualUnicodeString(&bn, &want, TRUE)) {
                    base = *(U64*)(e + 0x30);
                    break;
                }
            }
        }
    }
    KeUnstackDetachProcess(&apc);
    return base;
}

/* ============================ IL2CPP 链路 ============================ */

/* TypeInfo 槽 -> Il2CppClass*
 * 注意：槽在 .data 段里存的是指针，个别情况下存的是 RVA（低地址），两种都兜住 */
static U64 ClassFromTypeInfo(PEPROCESS proc, U64 rva)
{
    U64 slot = g_modBase + rva;
    U64 v = ReadPtr(proc, slot);
    if (!v) return 0;
    /* 如果读出来像 RVA（小于模块大小），当成类偏移 */
    if (v < g_modSize) v = g_modBase + v;
    return v;
}

static U64 StaticFields(PEPROCESS proc, U64 klass)
{
    if (!klass) return 0;
    return ReadPtr(proc, klass + OFF_CLASS_STATIC_FIELDS);
}

/* List<T> -> items 数组指针 + size */
static BOOLEAN ListInfo(PEPROCESS proc, U64 lst, U64* items, int* size)
{
    *items = 0; *size = 0;
    if (!lst) return FALSE;
    *items = ReadPtr(proc, lst + OFF_LIST_ITEMS);
    *size = ReadI32(proc, lst + OFF_LIST_SIZE);
    if (!*items || *size <= 0 || *size > 20000) return FALSE;
    return TRUE;
}

/* ============================ 采集 ============================ */

static U32 CollectBoids(PEPROCESS proc, RF4FEED* f)
{
    U64 klass, sf, lst, items;
    int n, i, cnt = 0;

    f->boidList = 0;
    f->classPtr = 0;
    klass = ClassFromTypeInfo(proc, RVA_TYPEINFO_MYBOID);
    if (!klass) return 0;
    f->classPtr = klass;

    sf = StaticFields(proc, klass);
    if (!sf) return 0;

    lst = ReadPtr(proc, sf + OFF_MYBOID_STATIC_LIST);
    if (!lst) return 0;
    f->boidList = lst;

    if (!ListInfo(proc, lst, &items, &n)) return 0;

    for (i = 0; i < n && cnt < MAX_ENT; i++) {
        U64 obj = ReadPtr(proc, items + OFF_ARRAY_FIRST + (U64)i * 8);
        float p[3];
        if (!obj) continue;
        if (!ReadVec3(proc, obj + g_posOff, p)) continue;
        if (!(p[0] > -50000.0f && p[0] < 50000.0f)) continue;
        if (!(p[1] > -50000.0f && p[1] < 50000.0f)) continue;
        if (!(p[2] > -50000.0f && p[2] < 50000.0f)) continue;
        if (p[0] > -1.0f && p[0] < 1.0f && p[1] > -1.0f && p[1] < 1.0f && p[2] > -1.0f && p[2] < 1.0f) continue;

        f->ent[cnt].x = p[0];
        f->ent[cnt].y = p[1];
        f->ent[cnt].z = p[2];
        f->ent[cnt].species = ReadI32(proc, obj + 0xD4);
        f->ent[cnt].state   = ReadI32(proc, obj + 0xD8);
        f->ent[cnt].flags   = 0;
        cnt++;
    }
    return (U32)cnt;
}

/* 连续发布线程 */
static VOID FeedThread(PVOID ctx)
{
    RF4FEED* f = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(RF4FEED), 'rf4f');
    UNREFERENCED_PARAMETER(ctx);
    if (!f) { K("[rf4] feed alloc fail\n"); PsTerminateSystemThread(STATUS_INSUFFICIENT_RESOURCES); }
    K("[rf4] feed thread up\n");
    for (;;) {
        if (!TGT_ALIVE()) { SleepMs(500); continue; }
        if (g_feedOn) {
            RtlZeroMemory(f, sizeof(RF4FEED));
            f->magic = FEED_MAGIC;
            f->version = FEED_VERSION;
            f->posOff = g_posOff;
            f->count = CollectBoids(g_proc, f);
            g_lastCount = f->count;
            g_lastList = f->boidList;
            g_lastClass = f->classPtr;
            WriteBufAt(FEED_FILE, f, sizeof(RF4FEED));
        }
        SleepMs(16);            /* ~60fps */
    }
}

/* ============================ 请求处理 ============================ */

static CHAR* HexOut(CHAR* p, CHAR* end, const U8* b, ULONG n)
{
    static const CHAR hx[] = "0123456789ABCDEF";
    ULONG i;
    for (i = 0; i < n && p + 3 < end; i++) {
        *p++ = hx[b[i] >> 4];
        *p++ = hx[b[i] & 0xF];
        *p++ = ' ';
    }
    if (p < end) *p = 0;
    return p;
}

static VOID DoProbe(PEPROCESS proc, U32 n, CHAR* out, ULONG cap, ULONG* used)
{
    CHAR* p = out, *end = out + cap - 1;
    U64 klass, sf, lst, items;
    int cnt, i;
    static const U32 offs[3] = { 0x78, 0x84, 0x90 };

    klass = ClassFromTypeInfo(proc, RVA_TYPEINFO_MYBOID);
    p = AppendFmt(p, end, "class=0x%llX\n", klass);
    sf = StaticFields(proc, klass);
    p = AppendFmt(p, end, "static=0x%llX\n", sf);
    if (!sf) { *used = (ULONG)(p - out); return; }
    lst = ReadPtr(proc, sf + OFF_MYBOID_STATIC_LIST);
    p = AppendFmt(p, end, "list=0x%llX\n", lst);
    if (!lst) { *used = (ULONG)(p - out); return; }
    if (!ListInfo(proc, lst, &items, &cnt)) {
        p = AppendFmt(p, end, "list invalid (items=0x%llX)\n", items);
        *used = (ULONG)(p - out); return;
    }
    p = AppendFmt(p, end, "items=0x%llX count=%d\n", items, cnt);
    for (i = 0; i < cnt && i < (int)n; i++) {
        U64 obj = ReadPtr(proc, items + OFF_ARRAY_FIRST + (U64)i * 8);
        float v0[3], v1[3], v2[3], vel[3];
        int k;
        p = AppendFmt(p, end, "[%d] obj=0x%llX\n", i, obj);
        ReadVec3(proc, obj + offs[0], v0);
        ReadVec3(proc, obj + offs[1], v1);
        ReadVec3(proc, obj + offs[2], v2);
        ReadVec3(proc, obj + 0xC8, vel);
        p = AppendFmt(p, end, "   +0x78 (%.2f,%.2f,%.2f)\n", v0[0], v0[1], v0[2]);
        p = AppendFmt(p, end, "   +0x84 (%.2f,%.2f,%.2f)\n", v1[0], v1[1], v1[2]);
        p = AppendFmt(p, end, "   +0x90 (%.2f,%.2f,%.2f)\n", v2[0], v2[1], v2[2]);
        p = AppendFmt(p, end, "   vel   (%.2f,%.2f,%.2f)\n", vel[0], vel[1], vel[2]);
        p = AppendFmt(p, end, "   species=%d state=%d vis=%d\n",
                      ReadI32(proc, obj + 0xD4), ReadI32(proc, obj + 0xD8),
                      ReadI32(proc, obj + 0x58) & 0xFF);
        for (k = 0; k < 6; k++) {
            float b;
            MemRead(proc, obj + 0x30 + (U64)k * 4, &b, 4);
            p = AppendFmt(p, end, "   b%d=%.1f", k, b);
        }
        p = AppendFmt(p, end, "\n");
        if (p > end - 400) break;
    }
    *used = (ULONG)(p - out);
}

static CHAR* AppendFmt(CHAR* p, CHAR* end, const CHAR* fmt, ...)
{
    va_list ap;
    size_t room;
    if (p >= end) return end;
    room = (size_t)(end - p);
    va_start(ap, fmt);
    RtlStringCchVPrintfA(p, room, fmt, ap);
    va_end(ap);
    p += strlen(p);
    return p;
}

static VOID ServeRequests(PEPROCESS proc)
{
    U8* req = ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x2000, 'reqb');
    CHAR* ack = ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x8000, 'ackb');
    ULONG hb = 0;
    if (!req || !ack) {
        if (req) ExFreePool(req);
        if (ack) ExFreePool(ack);
        return;
    }
    K("[rf4] serving requests\n");

    for (;;) {
        if (FilePresent(REQ_FILE)) {
            ULONG len = 0;
            CHAR* p, *le;
            ack[0] = 0;
            if (NT_SUCCESS(ReadAndDropRequest(req, 0x1FF0, &len)) && len) {
                p = (CHAR*)req;
                le = p + len;
                while (p < le && (*p == '\r' || *p == '\n' || *p == ' ')) p++;
                if (p >= le) { /* 空请求 */ }
                else if (p[0] == 's' && p[1] == 't') {
                    ULONG used = 0;
                    CHAR tmp[512];
                    CHAR* t = tmp;
                    t = AppendFmt(t, tmp + sizeof(tmp), "pid_alive=%d\n", TGT_ALIVE());
                    t = AppendFmt(t, tmp + sizeof(tmp), "module=0x%llX size=0x%X\n", g_modBase, g_modSize);
                    t = AppendFmt(t, tmp + sizeof(tmp), "posOff=0x%X feed=%u count=%u\n", g_posOff, g_feedOn, g_lastCount);
                    t = AppendFmt(t, tmp + sizeof(tmp), "class=0x%llX list=0x%llX\n", g_lastClass, g_lastList);
                    RtlStringCchCopyA(ack, 0x7FF0, tmp);
                    used = (ULONG)strlen(ack); UNREFERENCED_PARAMETER(used);
                }
                else if (p[0] == 'm') {
                    if (proc) {
                        U64 b = ModuleBase(proc, L"GameAssembly.dll");
                        RtlStringCchPrintfA(ack, 0x7FF0, "GameAssembly=0x%llX\n", b);
                    } else RtlStringCchCopyA(ack, 0x7FF0, "no process\n");
                }
                else if (p[0] == 'p' && p[1] == 'o' && p[2] == 's') {
                    ULONG off = 0; CHAR* q = p + 3;
                    while (q < le && *q == ' ') q++;
                    while (q < le) {
                        UCHAR c = *q;
                        if (c >= '0' && c <= '9') off = off * 16 + (c - '0');
                        else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f') off = off * 16 + ((c | 0x20) - 'a' + 10);
                        else break;
                        q++;
                    }
                    if (off < 0x2000) { g_posOff = off; }
                    RtlStringCchPrintfA(ack, 0x7FF0, "posOff=0x%X\n", g_posOff);
                }
                else if (p[0] == 'f' && p[1] == 'e') {
                    CHAR* q = p + 4;
                    while (q < le && *q == ' ') q++;
                    g_feedOn = (q < le && *q == '1') ? 1 : 0;
                    RtlStringCchPrintfA(ack, 0x7FF0, "feed=%u\n", g_feedOn);
                }
                else if (p[0] == 'p' && p[1] == 'r') {
                    ULONG n = 10, used = 0; CHAR* q = p + 5;
                    while (q < le && *q == ' ') q++;
                    if (q < le && *q >= '0' && *q <= '9') n = (ULONG)(*q - '0') * 10 + ((q + 1 < le && q[1] >= '0' && q[1] <= '9') ? (q[1] - '0') : 0);
                    if (n == 0) n = 10;
                    DoProbe(proc, n, ack, 0x7FF0, &used);
                }
                else if (p[0] == 'q' && p[1] == ' ') {
                    U64 va = 0; ULONG cnt = 8, i; CHAR* q = p + 2;
                    while (q < le && *q == ' ') q++;
                    if (q + 1 < le && q[0] == '0' && (q[1] == 'x' || q[1] == 'X')) q += 2;
                    while (q < le) {
                        UCHAR c = *q;
                        if (c >= '0' && c <= '9') va = va * 16 + (c - '0');
                        else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f') va = va * 16 + ((c | 0x20) - 'a' + 10);
                        else break;
                        q++;
                    }
                    while (q < le && *q == ' ') q++;
                    { ULONG t = 0; while (q < le && *q >= '0' && *q <= '9') { t = t * 10 + (*q - '0'); q++; } if (t) cnt = t; }
                    if (cnt > 64) cnt = 64;
                    { CHAR* o = ack;
                      for (i = 0; i < cnt; i++) {
                          U64 v = ReadPtr(proc, va + (U64)i * 8);
                          o = AppendFmt(o, ack + 0x7FF0, "+%03llX = 0x%llX\n", (U64)i * 8, v);
                      } }
                }
                else if (p[0] == 'r' && p[1] == ' ') {
                    U64 va = 0; ULONG sz = 0x40, got = 0; CHAR* q = p + 2;
                    U8 tmp[512];
                    while (q < le && *q == ' ') q++;
                    if (q + 1 < le && q[0] == '0' && (q[1] == 'x' || q[1] == 'X')) q += 2;
                    while (q < le) {
                        UCHAR c = *q;
                        if (c >= '0' && c <= '9') va = va * 16 + (c - '0');
                        else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f') va = va * 16 + ((c | 0x20) - 'a' + 10);
                        else break;
                        q++;
                    }
                    while (q < le && *q == ' ') q++;
                    { ULONG t = 0; while (q < le && *q >= '0' && *q <= '9') { t = t * 10 + (*q - '0'); q++; } if (t) sz = t; }
                    if (sz > sizeof(tmp)) sz = sizeof(tmp);
                    if (va && NT_SUCCESS(MemRead(proc, va, tmp, sz))) {
                        got = sz;
                        HexOut(ack, ack + 0x7FF0, tmp, got);
                        { CHAR* o = ack + strlen(ack);
                          o = AppendFmt(o, ack + 0x7FF0, "\ngot=%u\n", got); }
                    } else {
                        RtlStringCchPrintfA(ack, 0x7FF0, "read fail va=0x%llX size=%u\n", va, sz);
                    }
                }
                else if (p[0] == 'w' && p[1] == ' ') {
                    U64 va = 0; U8 buf[256]; ULONG bn = 0; CHAR* q = p + 2; int hi = -1;
                    while (q < le && *q == ' ') q++;
                    if (q + 1 < le && q[0] == '0' && (q[1] == 'x' || q[1] == 'X')) q += 2;
                    while (q < le) {
                        UCHAR c = *q;
                        if (c >= '0' && c <= '9') va = va * 16 + (c - '0');
                        else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f') va = va * 16 + ((c | 0x20) - 'a' + 10);
                        else break;
                        q++;
                    }
                    while (q < le && *q == ' ') q++;
                    while (q < le && bn < sizeof(buf)) {
                        UCHAR c = *q; int v;
                        if (c == ' ') { q++; continue; }
                        if (c >= '0' && c <= '9') v = c - '0';
                        else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f') v = (c | 0x20) - 'a' + 10;
                        else break;
                        if (hi < 0) hi = v; else { buf[bn++] = (U8)((hi << 4) | v); hi = -1; }
                        q++;
                    }
                    if (va && bn && NT_SUCCESS(MemWrite(proc, va, buf, bn)))
                        RtlStringCchPrintfA(ack, 0x7FF0, "wrote %u bytes at 0x%llX\n", bn, va);
                    else
                        RtlStringCchPrintfA(ack, 0x7FF0, "write fail\n");
                }
                else {
                    RtlStringCchCopyA(ack, 0x7FF0, "unknown command\n");
                }
                if (!ack[0]) RtlStringCchCopyA(ack, 0x7FF0, "done\n");
                WriteBufAt(ACK_FILE, ack, (ULONG)strlen(ack));
            }
        }
        SleepMs(50);
        if ((++hb % 600) == 0) K("[rf4] req loop alive\n");
    }
}

/* ============================ 等待与入口 ============================ */

static VOID Waiter(PVOID ctx)
{
    ULONG pid = 0, last = 0; ULONG waited = 0;
    UNREFERENCED_PARAMETER(ctx);
    K("[rf4] waiter alive\n");
    for (;;) {
        LARGE_INTEGER oc = {0};
        pid = FindPid(L"rf4_x64.exe", &oc);
        if (pid) break;
        SleepMs(500);
        waited += 500;
        if (waited > 3600000) { K("[rf4] wait timeout\n"); PsTerminateSystemThread(STATUS_TIMEOUT); }
    }
    KF("[rf4] game pid=%lu\n", pid);
    {
        NTSTATUS st = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &g_proc);
        if (!NT_SUCCESS(st)) { KF("[rf4] lookup=0x%X\n", st); PsTerminateSystemThread(st); }
    }
    g_modBase = ModuleBase(g_proc, L"GameAssembly.dll");
    /* 模块大小：读 PE 头 */
    if (g_modBase) {
        U32 e_lfanew = 0;
        if (NT_SUCCESS(MemRead(g_proc, g_modBase + 0x3C, &e_lfanew, 4))) {
            U32 opt = 0;
            if (NT_SUCCESS(MemRead(g_proc, g_modBase + e_lfanew + 0x18 + 0x38, &opt, 4)))
                g_modSize = opt;
        }
    }
    KF("[rf4] resident module=0x%llX size=0x%X\n", g_modBase, g_modSize);

    { HANDLE h = NULL;
      if (NT_SUCCESS(PsCreateSystemThread(&h, THREAD_ALL_ACCESS, NULL, NULL, NULL, FeedThread, NULL)) && h)
          ZwClose(h);
    }
    ServeRequests(g_proc);
    ObDereferenceObject(g_proc);
    PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT D, PUNICODE_STRING R)
{
    HANDLE h = NULL;
    NTSTATUS st;
    UNREFERENCED_PARAMETER(D); UNREFERENCED_PARAMETER(R);
    ExInitializeFastMutex(&g_mtx);
    K("\n[rf4] === entry ===\n");
    st = PsCreateSystemThread(&h, THREAD_ALL_ACCESS, NULL, NULL, NULL, Waiter, NULL);
    if (NT_SUCCESS(st) && h) ZwClose(h);
    { LARGE_INTEGER d; d.QuadPart = -30000000LL; KeDelayExecutionThread(KernelMode, FALSE, &d); }
    return STATUS_SUCCESS;
}
