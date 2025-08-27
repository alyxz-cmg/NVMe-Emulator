// main.cpp
#include "nvme_controller.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

using namespace NvmeSim;

void run_workload(NvmeController& ctrl, const std::string& name, int num_ops, double write_ratio, bool random_lba) {
    SystemMetrics metrics;
    uint64_t sim_time_us = 0;
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<> op_dist(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> lba_dist(0, ctrl.get_capacity() - 1);

    std::vector<uint64_t> latencies;
    latencies.reserve(num_ops);

    uint16_t current_cid = 0;
    int ops_submitted = 0;
    int ops_completed = 0;
    uint32_t seq_lba = 0;

    std::cout << "\n--- Running Workload: " << name << " ---\n";

    while (ops_completed < num_ops) {
        while (ops_submitted < num_ops) {
            Opcode op = (op_dist(gen) < write_ratio) ? Opcode::WRITE : Opcode::READ;
            uint32_t lba = random_lba ? lba_dist(gen) : (seq_lba++ % ctrl.get_capacity());

            NvmeCmd cmd = {current_cid, op, lba, 1, sim_time_us};

            if (ctrl.submit_command(cmd, metrics) == CommandStatus::QUEUE_FULL) {
                break;
            }
            current_cid++;
            ops_submitted++;
        }

        bool processed = ctrl.process_step(sim_time_us, metrics);

        if (!processed && ops_submitted < num_ops) {
            sim_time_us += 1;
        }

        while (auto cqe = ctrl.poll_completion()) {
            ops_completed++;
            metrics.total_commands++;
        }
    }

    double iops = (static_cast<double>(num_ops) / sim_time_us) * 1000000.0;
    double avg_latency = static_cast<double>(sim_time_us) / num_ops;

    std::cout << "Total Commands: " << metrics.total_commands << "\n";
    std::cout << "Simulated Time: " << sim_time_us / 1000 << " ms\n";
    std::cout << "IOPS:           " << iops << "\n";
    std::cout << "Avg Latency:    " << avg_latency << " us/cmd\n";
    std::cout << "SQ Backpressure:" << metrics.backpressure_events << " events\n";
    std::cout << "GC Events:      " << metrics.gc_events << "\n";
    std::cout << "Write Amplification: " << metrics.get_write_amplification() << "\n";
}

int main() {
    uint32_t QUEUE_DEPTH = 128;
    uint32_t DRIVE_CAPACITY = 100000;

    NvmeController ctrl(QUEUE_DEPTH, DRIVE_CAPACITY);

    std::cout << "NVMe + SSD Firmware Emulator Initialized.\n";
    std::cout << "Queue Depth: " << QUEUE_DEPTH << ", Capacity: " << DRIVE_CAPACITY << " blocks\n";

    run_workload(ctrl, "Sequential Reads", 100000, 0.0, false);

    run_workload(ctrl, "Random 70% Writes", 100000, 0.70, true);
    
    return 0;
}