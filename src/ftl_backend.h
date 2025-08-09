//ftl_backend.h
#pragma once
#include "nvme_types.h"
#include "metrics.h"
#include <vector>

namespace NvmeSim {

class FtlBackend {
public:
    explicit FtlBackend(uint32_t capacity)
        : capacity_(capacity),
          l2p_table_(capacity, UNMAPPED),
          free_blocks_(capacity * 1.2) {}

    uint64_t process_read(uint32_t lba) {
        if (lba >= capacity_) return 0;
        return 20;
    }

    uint64_t process_write(uint32_t lba, SystemMetrics& metrics) {
        if (lba >= capacity_) return 0;

        metrics.host_writes++;
        metrics.flash_writes++;

        free_blocks_--;
        l2p_table_[lba] = 1;

        return 100;
    }

    uint64_t process_trim(uint32_t lba) {
        if (lba < capacity_ && l2p_table_[lba] != UNMAPPED) {
            l2p_table_[lba] = UNMAPPED;
            free_blocks_++;
        }
        return 10;
    }

    uint64_t process_flush() { return 500; }

private:
    static constexpr uint32_t UNMAPPED = 0xFFFFFFFF;

    uint32_t capacity_;
    int32_t free_blocks_;
    std::vector<uint32_t> l2p_table_;
};
}