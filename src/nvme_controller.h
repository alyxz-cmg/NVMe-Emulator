//nvme_controller.h
#pragma once
#include "nvme_types.h"
#include "ftl_backend.h"
#include <deque>

namespace NvmeSim {

class NvmeController {
public:
    NvmeController(uint32_t depth, uint32_t cap)
        : depth_(depth), ftl_(cap) {}

    CommandStatus submit_command(const NvmeCmd& cmd, SystemMetrics& m) {
        if (sq_.size() >= depth_) {
            m.backpressure_events++;
            
            return CommandStatus::QUEUE_FULL;
        }

        sq_.push_back(cmd);

        return CommandStatus::SUCCESS;
    }

private:
    uint32_t depth_;
    std::deque<NvmeCmd> sq_;
    FtlBackend ftl_;
};
}