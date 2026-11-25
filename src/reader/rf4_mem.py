"""rf4_mem.py —— Russian Fishing 4 内存读取基础库

只读。不需要驱动，用户态 OpenProcess + ReadProcessMemory 即可。

用法:
    from rf4_mem import RF4
    g = RF4()
    print(hex(g.mod_base), g.mod_size)
    cls = g.il2cpp_class(0x040C7FE8)      # MyBoid 的 TypeInfo 槽
    sf  = g.static_fields(cls)
    lst = g.ptr(sf + 0x0)                 # static List<MyBoid>
    for obj in g.list_items(lst):
        ...
"""
import ctypes
import ctypes.wintypes as wt
import struct

kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ = 0x0010

TH32CS_SNAPPROCESS = 0x00000002
TH32CS_SNAPMODULE = 0x00000008
TH32CS_SNAPMODULE32 = 0x00000010

# Il2CppClass 固定偏移（Health.txt 实测确认）
IL2CPP_CLASS_NAME = 0x10
IL2CPP_CLASS_NAMESPACE = 0x18
IL2CPP_CLASS_STATIC_FIELDS = 0xB8

LIST_ITEMS = 0x10
LIST_SIZE = 0x18
ARRAY_LENGTH = 0x18
ARRAY_FIRST = 0x20

OBJECT_HEADER = 0x10


class PROCESSENTRY32(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD), ("cntUsage", wt.DWORD),
        ("th32ProcessID", wt.DWORD), ("th32DefaultHeapID", ctypes.POINTER(ctypes.c_ulong)),
        ("th32ModuleID", wt.DWORD), ("cntThreads", wt.DWORD),
        ("th32ParentProcessID", wt.DWORD), ("pcPriClassBase", ctypes.c_long),
        ("dwFlags", wt.DWORD), ("szExeFile", ctypes.c_char * 260),
    ]


class MODULEENTRY32(ctypes.Structure):
    _fields_ = [
        ("dwSize", wt.DWORD), ("th32ModuleID", wt.DWORD),
        ("th32ProcessID", wt.DWORD), ("GlblcntUsage", wt.DWORD),
        ("ProccntUsage", wt.DWORD), ("modBaseAddr", ctypes.POINTER(ctypes.c_byte)),
        ("modBaseSize", wt.DWORD), ("hModule", wt.HMODULE),
        ("szModule", ctypes.c_char * 256), ("szExePath", ctypes.c_char * 260),
    ]


def find_pid(name="rf4_x64.exe"):
    snap = kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    if snap == -1:
        raise OSError("CreateToolhelp32Snapshot 失败")
    try:
        e = PROCESSENTRY32()
        e.dwSize = ctypes.sizeof(PROCESSENTRY32)
        ok = kernel32.Process32First(snap, ctypes.byref(e))
        target = name.lower()
        while ok:
            if e.szExeFile.decode("mbcs", "ignore").lower() == target:
                return e.th32ProcessID
            ok = kernel32.Process32Next(snap, ctypes.byref(e))
    finally:
        kernel32.CloseHandle(snap)
    return None


class RF4:
    def __init__(self, exe="rf4_x64.exe", module="GameAssembly.dll"):
        pid = find_pid(exe)
        if not pid:
            raise RuntimeError("找不到进程 %s，游戏没运行？" % exe)
        self.pid = pid
        self.h = kernel32.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, False, pid)
        if not self.h:
            raise RuntimeError("OpenProcess 失败，错误码 %d（需要管理员权限？）" % ctypes.get_last_error())
        self.mod_base, self.mod_size = self.module(module)
        if not self.mod_base:
            raise RuntimeError("找不到模块 %s" % module)

    # ---------- 基础 IO ----------
    def read(self, addr, size):
        buf = ctypes.create_string_buffer(size)
        got = ctypes.c_size_t(0)
        ok = kernel32.ReadProcessMemory(self.h, ctypes.c_void_p(addr), buf, size, ctypes.byref(got))
        if not ok or got.value != size:
            return None
        return buf.raw

    def ptr(self, addr):
        b = self.read(addr, 8)
        return struct.unpack("<Q", b)[0] if b else 0

    def u32(self, addr):
        b = self.read(addr, 4)
        return struct.unpack("<I", b)[0] if b else 0

    def i32(self, addr):
        b = self.read(addr, 4)
        return struct.unpack("<i", b)[0] if b else 0

    def f32(self, addr):
        b = self.read(addr, 4)
        return struct.unpack("<f", b)[0] if b else float("nan")

    def f64(self, addr):
        b = self.read(addr, 8)
        return struct.unpack("<d", b)[0] if b else float("nan")

    def vec3(self, addr):
        b = self.read(addr, 12)
        return struct.unpack("<3f", b) if b else (float("nan"),) * 3

    def cstr(self, addr, maxlen=128):
        b = self.read(addr, maxlen)
        if not b:
            return ""
        return b.split(b"\x00")[0].decode("utf-8", "replace")

    # ---------- 模块 ----------
    def module(self, name):
        snap = kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, self.pid)
        if snap == -1:
            return 0, 0
        try:
            m = MODULEENTRY32()
            m.dwSize = ctypes.sizeof(MODULEENTRY32)
            ok = kernel32.Module32First(snap, ctypes.byref(m))
            target = name.lower()
            while ok:
                if m.szModule.decode("mbcs", "ignore").lower() == target:
                    return ctypes.cast(m.modBaseAddr, ctypes.c_void_p).value, m.modBaseSize
                ok = kernel32.Module32Next(snap, ctypes.byref(m))
        finally:
            kernel32.CloseHandle(snap)
        return 0, 0

    # ---------- IL2CPP ----------
    def il2cpp_class(self, typeinfo_rva):
        """TypeInfo 槽 → Il2CppClass*（槽里存的就是指针）"""
        return self.ptr(self.mod_base + typeinfo_rva)

    def static_fields(self, klass):
        return self.ptr(klass + IL2CPP_CLASS_STATIC_FIELDS)

    def class_name(self, klass):
        return self.cstr(self.ptr(klass + IL2CPP_CLASS_NAME))

    def static_ptr(self, typeinfo_rva, field_offset):
        """一步到位：TypeInfo RVA + 静态字段偏移 → 实例指针"""
        cls = self.il2cpp_class(typeinfo_rva)
        if not cls:
            return 0
        sf = self.static_fields(cls)
        if not sf:
            return 0
        return self.ptr(sf + field_offset)

    def list_items(self, lst):
        """List<T> → 元素指针列表"""
        if not lst:
            return []
        items = self.ptr(lst + LIST_ITEMS)
        size = self.i32(lst + LIST_SIZE)
        if not items or size <= 0 or size > 100000:
            return []
        out = []
        for i in range(size):
            p = self.ptr(items + ARRAY_FIRST + i * 8)
            if p:
                out.append(p)
        return out

    def close(self):
        if self.h:
            kernel32.CloseHandle(self.h)
            self.h = None
