// tracer/tracer.h
#pragma once

#include <cinttypes>   // PRId64
#include <cstddef>
#include <atomic>

// A single recorded call event
struct CallEvent {
    const void* fn_addr;    // raw function pointer
    const void* call_site;  // address of the call instruction
    bool        is_entry;   // true = enter, false = exit
    int64_t     ts_ns;      // monotonic timestamp in nanoseconds
};

// Fixed-size trace buffer — 2M events (~64 MB)
// Uses a lock-free ring buffer with atomic counter for thread-safety
constexpr std::size_t TRACE_BUFFER_SIZE = 2 * 1024 * 1024;

// Clear the trace (call before each test)
void trace_reset();

// Dump the current trace to a JSON file at the given path.
// Resolves function names via dladdr + demangling.
void trace_dump_json(const char* output_path);
