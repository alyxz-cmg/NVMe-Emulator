//nvme_types.h
#pragma once
#include <cstdint>

namespace NvmeSim {

enum class Opcode : uint8_t {
    READ = 0x02,
    WRITE = 0x01,
    TRIM = 0x09,
    FLUSH = 0x00
};

enum class CommandStatus : uint8_t {
    SUCCESS = 0x00,
    FAILURE = 0x01,
    QUEUE_FULL = 0x02,
    INVALID_LBA = 0x03
};

struct NvmeCmd {
    uint16_t cid;
    Opcode opcode;
    uint32_t lba;
    uint32_t length;
    uint64_t submit_time;
};

struct NvmeCqe {
    uint16_t cid;
    CommandStatus status;
    uint64_t completion_time;
};
}