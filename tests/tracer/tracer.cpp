// tracer/tracer.cpp
#include "tracer.h"

#include <atomic>
#include <cinttypes>    // PRId64
#include <cstddef>      // size_t
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cxxabi.h>     // abi::__cxa_demangle
#include <dlfcn.h>      // dladdr
#include <string>
#include <unordered_map>

// Fixed-size trace buffer — 2M events (~64 MB)
// Using a simple lock-free ring buffer to avoid STL in instrumented code
static CallEvent g_trace_buffer[TRACE_BUFFER_SIZE];
static std::atomic<std::size_t> g_trace_idx{0};

static inline int64_t now_ns() __attribute__((no_instrument_function));
static inline int64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

extern "C" {

// __attribute__((no_instrument_function)) prevents recursion
void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void* fn, void* call_site) {
    size_t idx = g_trace_idx.fetch_add(1, std::memory_order_relaxed);
    if (idx < TRACE_BUFFER_SIZE) {
        g_trace_buffer[idx] = { fn, call_site, true, now_ns() };
    }
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void* fn, void* call_site) {
    size_t idx = g_trace_idx.fetch_add(1, std::memory_order_relaxed);
    if (idx < TRACE_BUFFER_SIZE) {
        g_trace_buffer[idx] = { fn, call_site, false, now_ns() };
    }
}

} // extern "C"

void trace_reset() {
    g_trace_idx.store(0, std::memory_order_relaxed);
}

// Resolve fn_addr to a JSON-safe display name; cache by address to avoid
// repeated dladdr + demangle (was the main bottleneck with 100k+ events).
static std::string resolve_and_escape_name(const void* fn_addr,
                                           std::unordered_map<const void*, std::string>& cache) {
    auto it = cache.find(fn_addr);
    if (it != cache.end())
        return it->second;

    Dl_info info;
    memset(&info, 0, sizeof(info));
    bool resolved = (dladdr(fn_addr, &info) != 0);
    const char* raw_name = (resolved && info.dli_sname && info.dli_sname[0])
                           ? info.dli_sname : "??";

    int status = -1;
    char* demangled = abi::__cxa_demangle(raw_name, nullptr, nullptr, &status);
    const char* display_name = (status == 0 && demangled) ? demangled : raw_name;

    std::string safe_name;
    for (const char* p = display_name; *p; p++) {
        unsigned char c = static_cast<unsigned char>(*p);
        if      (c == '"')  safe_name += "\\\"";
        else if (c == '\\') safe_name += "\\\\";
        else if (c == '\n') safe_name += "\\n";
        else if (c == '\r') safe_name += "\\r";
        else if (c == '\t') safe_name += "\\t";
        else if (c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            safe_name += buf;
        } else {
            safe_name += static_cast<char>(c);
        }
    }
    free(demangled);

    cache[fn_addr] = safe_name;
    return safe_name;
}

void trace_dump_json(const char* output_path) {
    FILE* f = fopen(output_path, "w");
    if (!f) {
        fprintf(stderr, "[tracer] Failed to open output: %s\n", output_path);
        return;
    }

    size_t count = g_trace_idx.load(std::memory_order_relaxed);
    if (count > TRACE_BUFFER_SIZE) count = TRACE_BUFFER_SIZE;

    std::unordered_map<const void*, std::string> name_cache;
    name_cache.reserve(4096);  // typical unique function count per test

    fprintf(f, "[\n");

    for (size_t i = 0; i < count; i++) {
        const CallEvent& e = g_trace_buffer[i];
        const std::string& safe_name = resolve_and_escape_name(e.fn_addr, name_cache);

        fprintf(f,
            "  {\"type\":\"%s\",\"fn\":\"%s\",\"addr\":\"%p\",\"ts_ns\":%" PRId64 "}%s\n",
            e.is_entry ? "enter" : "exit",
            safe_name.c_str(),
            e.fn_addr,
            e.ts_ns,
            (i + 1 < count) ? "," : ""
        );
    }

    fprintf(f, "]\n");
    fclose(f);

    fprintf(stdout, "[tracer] Wrote %zu events to %s\n", count, output_path);
}
