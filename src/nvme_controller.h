//nvme_controller.h
#pragma once
#include "nvme_types.h"
#include "ftl_backend.h"
#include <deque>
#include <optional>

namespace NvmeSim {

class NvmeController {
public:
    NvmeController(uint32_t queue_depth, uint32_t logical_capacity)
        : queue_depth_(queue_depth), ftl_(logical_capacity) {}

    CommandStatus submit_command(const NvmeCmd& cmd, SystemMetrics& metrics) {
        if (sq_.size() >= queue_depth_) {
            metrics.backpressure_events++;
            return CommandStatus::QUEUE_FULL;
        }
        sq_.push_back(cmd);
        return CommandStatus::SUCCESS;
    }

    std::optional<NvmeCqe> poll_completion() {
        if (cq_.empty()) return std::nullopt;
        NvmeCqe cqe = cq_.front();
        cq_.pop_front();
        return cqe;
    }

    bool process_step(uint64_t& current_sim_time_us, SystemMetrics& metrics) {
        if (sq_.empty()) return false;

        NvmeCmd cmd = sq_.front();
        sq_.pop_front();

        uint64_t op_latency = 0;
        CommandStatus status = CommandStatus::SUCCESS;

        switch (cmd.opcode) {
            case Opcode::READ:
                op_latency = ftl_.process_read(cmd.lba);
                break;
            case Opcode::WRITE:
                op_latency = ftl_.process_write(cmd.lba, metrics);
                break;
            case Opcode::TRIM:
                op_latency = ftl_.process_trim(cmd.lba);
                break;
            case Opcode::FLUSH:
                op_latency = ftl_.process_flush();
                break;
        }

        if (op_latency == 0 && cmd.opcode != Opcode::FLUSH) {
            status = CommandStatus::INVALID_LBA;
            op_latency = 5;
        }

        current_sim_time_us += op_latency;

        cq_.push_back({cmd.cid, status, current_sim_time_us});
        return true;
    }

    uint32_t get_capacity() const { 
        return ftl_.get_capacity();
    }

private:
    uint32_t queue_depth_;
    std::deque<NvmeCmd> sq_;
    std::deque<NvmeCqe> cq_;
    FtlBackend ftl_;
};
}