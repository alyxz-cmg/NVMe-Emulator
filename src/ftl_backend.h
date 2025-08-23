//ftl_backend.h
#pragma once
#include "nvme_types.h"
#include "metrics.h"
#include <vector>
#include <unordered_map>
#include <iostream>

namespace NvmeSim {

class FtlBackend {
public:
    FtlBackend(uint32_t logical_capacity) 
        : capacity_(logical_capacity), free_blocks_(logical_capacity * 1.2) {
        l2p_table_.resize(capacity_, UNMAPPED);
    }

    uint64_t process_read(uint32_t lba) {
        if (lba >= capacity_) return 0;
        return LATENCY_READ_US;
    }

    uint64_t process_write(uint32_t lba, SystemMetrics& metrics) {
        if (lba >= capacity_) return 0;
        
        metrics.host_writes++;
        metrics.flash_writes++;
        uint64_t latency = LATENCY_WRITE_US;

        if (l2p_table_[lba] != UNMAPPED) {
            free_blocks_++;
        }
        
        free_blocks_--;
        l2p_table_[lba] = 1;

        if (free_blocks_ < capacity_ * 0.05) {
            latency += trigger_gc(metrics);
        }

        return latency;
    }

    uint64_t process_trim(uint32_t lba) {
        if (lba < capacity_ && l2p_table_[lba] != UNMAPPED) {
            l2p_table_[lba] = UNMAPPED;
            free_blocks_++;
        }
        return LATENCY_TRIM_US;
    }

    uint64_t process_flush() {
        return LATENCY_FLUSH_US;
    }

    uint32_t get_capacity() const { return capacity_; }

private:
    static constexpr uint32_t UNMAPPED = 0xFFFFFFFF;
    static constexpr uint64_t LATENCY_READ_US = 20;
    static constexpr uint64_t LATENCY_WRITE_US = 100;
    static constexpr uint64_t LATENCY_TRIM_US = 10;
    static constexpr uint64_t LATENCY_FLUSH_US = 500;
    static constexpr uint64_t LATENCY_GC_BLOCK_ERASE = 1500;

    uint32_t capacity_;
    int32_t free_blocks_;
    std::vector<uint32_t> l2p_table_;

    uint64_t trigger_gc(SystemMetrics& metrics) {
        metrics.gc_events++;
        uint64_t pages_moved = capacity_ * 0.02;
        metrics.flash_writes += pages_moved;
        free_blocks_ += (capacity_ * 0.10);
        
        return (pages_moved * LATENCY_WRITE_US) + LATENCY_GC_BLOCK_ERASE;
    }
};
}