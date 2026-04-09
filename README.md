# NVMe Command Queue and SSD Firmware Emulator

## Overview

This project is a software-based simulation of an NVMe storage system and underlying SSD firmware behavior. It models the interaction between a host system and a simplified SSD through an NVMe controller abstraction, including submission queues (SQ), completion queues (CQ), and a backend flash translation layer (FTL).

The goal of this project is to demonstrate how NVMe command processing, firmware scheduling, and flash storage constraints interact under realistic workloads, including latency modeling, queue backpressure, and garbage collection behavior.

---

## Architecture

The system is composed of four main layers:

### 1. NVMe Command Layer
Defines NVMe-like commands and completion semantics.
* READ, WRITE, TRIM, FLUSH opcodes
* Submission Queue Entry (`NvmeCmd`)
* Completion Queue Entry (`NvmeCqe`)
* Command status tracking

### 2. NVMe Controller
Simulates firmware-level command execution.
**Responsibilities:**
* Accept commands via a submission queue
* Enforce queue depth limits (backpressure simulation)
* Process commands sequentially
* Return completions via a completion queue
* Maintain simulated time progression

### 3. FTL Backend
Implements a simplified Flash Translation Layer.
**Responsibilities:**
* Logical-to-physical mapping (L2P table)
* Simulated NAND latency for reads and writes
* TRIM support
* Garbage collection trigger mechanism
* Write amplification tracking

### 4. System Metrics
Collects runtime performance statistics.
**Tracked metrics:**
* Total commands processed
* Host and flash write counts
* GC event count
* Backpressure events
* Write amplification ratio

---

## Workload Model

Two workloads are used to evaluate system behavior:

### 1. Sequential Reads
* 100,000 commands
* 100% read workload
* Sequential LBA access pattern

### 2. Random 70% Write Workload
* 100,000 commands
* 70% writes, 30% reads
* Random LBA selection

---

## Results Summary

| Metric | Sequential Reads | Random 70% Writes |
| :--- | :--- | :--- |
| **Total Commands** | 100,000 | 100,000 |
| **Simulated Time** | 2,000 ms | 7,595 ms |
| **IOPS** | 50,000 | 13,165 |
| **Avg Latency** | 20 us/cmd | 75.95 us/cmd |
| **SQ Backpressure** | 99,872 events | 99,872 events |
| **GC Events** | 0 | 0 |
| **Write Amplification** | 1.0 | 1.0 |

---

## Key Observations

### 1. Backpressure Behavior
The system exhibits a high number of submission queue backpressure events due to continuous queue saturation under sustained workload generation. This reflects a simplified controller model without parallel command execution or deep pipelining.

### 2. Garbage Collection
No garbage collection events were triggered in the current run. This indicates that the free space model remains above the GC threshold throughout execution, or GC conditions were not sufficiently stressed in this configuration.

### 3. Write Amplification
Write amplification remains at 1.0 in both workloads. This is expected given the simplified FTL model, which does not yet simulate realistic page-level invalidation or multi-write relocation behavior.

### 4. Latency Scaling
The random write workload shows significantly higher latency compared to sequential reads, reflecting increased simulation overhead and access pattern randomness.

---

## Limitations

This is an educational simulator and does not implement:
* Parallel NVMe submission/completion queues
* Out-of-order completion
* Real NAND block/page-level management
* Wear leveling
* Detailed garbage collection algorithms
* DRAM caching of mapping tables
* Multi-plane or multi-die flash behavior

---

## Build Instructions

### Compile (g++)
```bash
g++ src/*.cpp -o nvme_sim -O2
```

### Run
```bash
./nvme_sim
```

---

## Project Goals
This project is designed to demonstrate:
* Understanding of NVMe architecture
* Firmware-level storage system design
* Flash memory constraints and abstractions
* Performance modeling and system metrics tracking
* Event-driven simulation of storage pipelines

## Future Improvements
Planned extensions include:
* Parallel submission queues (multi-core simulation)
* Realistic garbage collection with block-level wear tracking
* Page-level mapping with valid/invalid state tracking
* P99 latency measurement and histograms
* Interrupt-driven completion model
* DRAM-resident L2P caching layer
* Wear leveling algorithms
