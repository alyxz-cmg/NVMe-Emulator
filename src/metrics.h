//metrics.h
#pragma once
#include <cstdint>

namespace NvmeSim {

struct SystemMetrics {
    uint64_t total_commands = 0;
    uint64_t total_latency_us = 0;
    uint64_t max_latency_us = 0;
    uint64_t host_writes = 0;
    uint64_t flash_writes = 0;
    uint64_t gc_events = 0;
    uint64_t backpressure_events = 0;

    double get_write_amplification() const {
        return host_writes == 0 ? 1.0 :
            static_cast<double>(flash_writes) / host_writes;
    }
};
}