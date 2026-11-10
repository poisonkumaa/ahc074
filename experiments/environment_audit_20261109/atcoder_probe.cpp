// Environment diagnostic for AtCoder Code Test; this is not an AHC074 solution.
// No input, network access, environment-variable dump, hostname, or username.
// Intentionally no global optimization pragma: inspect the selected language's
// defaults, with separately attributed functions for the FP comparison below.
#include <algorithm>
#include <array>
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sched.h>
#include <unistd.h>
#if defined(__GLIBC__)
#include <gnu/libc-version.h>
#endif
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#include <xmmintrin.h>
#endif

template<class T> void print(const char* name, const T& value) {
    std::cout << name << '=' << value << '\n';
}

void file_value(const char* name, const char* path) {
    std::ifstream f(path);
    if (!f) { print(name, "unavailable"); return; }
    std::string line, value;
    while (std::getline(f, line)) {
        if (!value.empty()) value += " | ";
        value += line;
        if (value.size() > 4096) { value.resize(4096); break; }
    }
    print(name, value);
}

void limit_value(const char* name, int kind) {
    rlimit r{};
    if (getrlimit(kind, &r)) { print(name, "unavailable"); return; }
    auto value = [](rlim_t x) {
        return x == RLIM_INFINITY ? std::string("unlimited") : std::to_string(x);
    };
    print(name, "soft:" + value(r.rlim_cur) + ",hard:" + value(r.rlim_max));
}

void resolution(const char* name, clockid_t kind) {
    timespec r{};
    if (clock_getres(kind, &r)) { print(name, "unavailable"); return; }
    print(name, std::to_string(r.tv_sec) + "s+" + std::to_string(r.tv_nsec) + "ns");
}

#if defined(__GNUC__) && !defined(__clang__)
#define FP_FUNCTION(level) __attribute__((noinline,noclone,optimize(level)))
#else
#define FP_FUNCTION(level) __attribute__((noinline))
#endif
FP_FUNCTION("O1") double multiply_add_o1(double a, double b, double c) { return a*b+c; }
FP_FUNCTION("O3") double multiply_add_o3(double a, double b, double c) { return a*b+c; }
FP_FUNCTION("O3") double multiply_add_fused(double a, double b, double c) { return std::fma(a,b,c); }
FP_FUNCTION("O3") double multiply_add_separate(double a, double b, double c) {
    volatile double product = a*b;
    return product+c;
}

int main() {
    print("probe_version", 1);
    print("compiler.version", __VERSION__);
    print("language.__cplusplus", __cplusplus);
#if defined(__GNUC__)
    print("compiler.gcc", std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__));
#endif
#if defined(__clang__)
    print("compiler.clang", __clang_version__);
#endif
#ifdef __GLIBCXX__
    print("stdlib.__GLIBCXX__", __GLIBCXX__);
#endif
#ifdef _GLIBCXX_RELEASE
    print("stdlib._GLIBCXX_RELEASE", _GLIBCXX_RELEASE);
#endif
#ifdef _GLIBCXX_USE_CXX11_ABI
    print("stdlib._GLIBCXX_USE_CXX11_ABI", _GLIBCXX_USE_CXX11_ABI);
#endif
#ifdef __GLIBC__
    print("libc.headers", std::to_string(__GLIBC__) + "." + std::to_string(__GLIBC_MINOR__));
    print("libc.runtime", gnu_get_libc_version());
#endif
#define REPORT_FLAG(name) print("macro." #name, name)
#ifdef ONLINE_JUDGE
    REPORT_FLAG(ONLINE_JUDGE);
#else
    print("macro.ONLINE_JUDGE", "undefined");
#endif
#ifdef ATCODER
    REPORT_FLAG(ATCODER);
#else
    print("macro.ATCODER", "undefined");
#endif
#ifdef __OPTIMIZE__
    REPORT_FLAG(__OPTIMIZE__);
#else
    print("macro.__OPTIMIZE__", "undefined");
#endif
#ifdef __FAST_MATH__
    REPORT_FLAG(__FAST_MATH__);
#else
    print("macro.__FAST_MATH__", "undefined");
#endif
#ifdef __FINITE_MATH_ONLY__
    REPORT_FLAG(__FINITE_MATH_ONLY__);
#endif
#ifdef NDEBUG
    print("macro.NDEBUG", "defined");
#else
    print("macro.NDEBUG", "undefined");
#endif
#ifdef _GLIBCXX_ASSERTIONS
    print("macro._GLIBCXX_ASSERTIONS", "defined");
#else
    print("macro._GLIBCXX_ASSERTIONS", "undefined");
#endif
#ifdef __FMA__
    REPORT_FLAG(__FMA__);
#else
    print("macro.__FMA__", "undefined");
#endif
#ifdef __AVX__
    REPORT_FLAG(__AVX__);
#else
    print("macro.__AVX__", "undefined");
#endif
#ifdef __AVX2__
    REPORT_FLAG(__AVX2__);
#else
    print("macro.__AVX2__", "undefined");
#endif
#ifdef __POPCNT__
    REPORT_FLAG(__POPCNT__);
#else
    print("macro.__POPCNT__", "undefined");
#endif
#ifdef __BMI__
    REPORT_FLAG(__BMI__);
#else
    print("macro.__BMI__", "undefined");
#endif
#ifdef __BMI2__
    REPORT_FLAG(__BMI2__);
#else
    print("macro.__BMI2__", "undefined");
#endif
#ifdef __AVX512F__
    REPORT_FLAG(__AVX512F__);
#else
    print("macro.__AVX512F__", "undefined");
#endif
#ifdef __SSE2_MATH__
    REPORT_FLAG(__SSE2_MATH__);
#endif
#ifdef __FLT_EVAL_METHOD__
    REPORT_FLAG(__FLT_EVAL_METHOD__);
#endif
    print("compiler.flag_note", "macro flags describe the translation-unit defaults; the complete command and -O level cannot be recovered from them");

    utsname u{};
    if (!uname(&u)) {
        print("os.sysname", u.sysname);
        print("os.release", u.release);
        print("os.version", u.version);
        print("os.machine", u.machine);
    }
    // nodename is deliberately omitted.
    print("abi.pointer_bytes", sizeof(void*));
    print("abi.int_bytes", sizeof(int));
    print("abi.long_bytes", sizeof(long));
    print("abi.long_double_bytes", sizeof(long double));
    print("abi.double_iec559", std::numeric_limits<double>::is_iec559);
    print("abi.byte_order", std::endian::native == std::endian::little ? "little" : "other");
    print("clock.CLOCKS_PER_SEC", CLOCKS_PER_SEC);
    resolution("clock.monotonic_resolution", CLOCK_MONOTONIC);
    resolution("clock.process_cpu_resolution", CLOCK_PROCESS_CPUTIME_ID);
    resolution("clock.thread_cpu_resolution", CLOCK_THREAD_CPUTIME_ID);
    print("clock.sysconf_CLK_TCK", sysconf(_SC_CLK_TCK));

    print("cpu.online_count", sysconf(_SC_NPROCESSORS_ONLN));
    print("cpu.hardware_concurrency", std::thread::hardware_concurrency());
    cpu_set_t affinity;
    CPU_ZERO(&affinity);
    if (!sched_getaffinity(0, sizeof(affinity), &affinity)) {
        print("cpu.affinity_count", CPU_COUNT(&affinity));
        std::string cpus;
        for (int i=0; i<CPU_SETSIZE; ++i) if (CPU_ISSET(i, &affinity)) {
            if (!cpus.empty()) cpus += ',';
            cpus += std::to_string(i);
        }
        print("cpu.affinity_ids", cpus);
    } else print("cpu.affinity_count", "unavailable");
#if defined(__x86_64__) || defined(__i386__)
    unsigned a=0,b=0,c=0,d=0;
    if (__get_cpuid(0, &a, &b, &c, &d)) {
        char vendor[13]{};
        std::memcpy(vendor, &b, 4); std::memcpy(vendor+4, &d, 4); std::memcpy(vendor+8, &c, 4);
        print("cpu.cpuid_vendor", vendor);
    }
    if (__get_cpuid_max(0x80000000, nullptr) >= 0x80000004) {
        char brand[49]{};
        for (unsigned leaf=0x80000002; leaf<=0x80000004; ++leaf) {
            __get_cpuid(leaf, &a, &b, &c, &d);
            const unsigned words[4]={a,b,c,d};
            std::memcpy(brand+16*(leaf-0x80000002), words, 16);
        }
        print("cpu.cpuid_brand", brand);
    }
    __builtin_cpu_init();
#define CPU_FEATURE(name) print("cpu.runtime." name, bool(__builtin_cpu_supports(name)))
    CPU_FEATURE("sse2"); CPU_FEATURE("sse4.2"); CPU_FEATURE("avx");
    CPU_FEATURE("avx2"); CPU_FEATURE("fma"); CPU_FEATURE("popcnt");
    CPU_FEATURE("bmi"); CPU_FEATURE("bmi2"); CPU_FEATURE("avx512f");
    CPU_FEATURE("avx512bw"); CPU_FEATURE("avx512vl");
    const unsigned mxcsr=_mm_getcsr();
    print("fp.MXCSR", mxcsr);
    print("fp.MXCSR_flush_to_zero", bool(mxcsr & (1u<<15)));
    print("fp.MXCSR_denormals_are_zero", bool(mxcsr & (1u<<6)));
#endif
    const int rounding=std::fegetround();
    print("fp.rounding", rounding==FE_TONEAREST ? "nearest" : rounding==FE_DOWNWARD ? "downward" : rounding==FE_UPWARD ? "upward" : rounding==FE_TOWARDZERO ? "towardzero" : "unknown");
    volatile double va=1.0+0x1p-27, vb=1.0-0x1p-27, vc=-1.0;
    const double fa=va,fb=vb,fc=vc;
    std::cout << std::hexfloat;
    print("fp.a_times_b_plus_c.O1", multiply_add_o1(fa,fb,fc));
    print("fp.a_times_b_plus_c.O3", multiply_add_o3(fa,fb,fc));
    print("fp.a_times_b_plus_c.std_fma", multiply_add_fused(fa,fb,fc));
    print("fp.a_times_b_plus_c.separate", multiply_add_separate(fa,fb,fc));
    std::cout << std::defaultfloat;

    limit_value("limit.cpu_seconds", RLIMIT_CPU);
    limit_value("limit.address_space_bytes", RLIMIT_AS);
    limit_value("limit.stack_bytes", RLIMIT_STACK);
    limit_value("limit.open_files", RLIMIT_NOFILE);
    file_value("cgroup.v2.cpu_max", "/sys/fs/cgroup/cpu.max");
    file_value("cgroup.v2.memory_max", "/sys/fs/cgroup/memory.max");
    file_value("cgroup.v2.cpuset_cpus_effective", "/sys/fs/cgroup/cpuset.cpus.effective");
    file_value("cgroup.v1.cpu_quota_us", "/sys/fs/cgroup/cpu/cpu.cfs_quota_us");
    file_value("cgroup.v1.cpu_period_us", "/sys/fs/cgroup/cpu/cpu.cfs_period_us");
    file_value("cgroup.v1.memory_limit_bytes", "/sys/fs/cgroup/memory/memory.limit_in_bytes");
    print("limit.note", "visible limits may not include external judge enforcement or stricter ancestor cgroups; unavailable does not mean unlimited");
}
