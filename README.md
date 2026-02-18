# Parallel Firewall — Producer-Consumer with Ring Buffer in C

A **multithreaded network packet firewall** implemented in C using POSIX threads. A producer thread reads packets from a binary file and enqueues them into a **thread-safe ring buffer**; multiple consumer threads dequeue and filter packets concurrently, writing PASS/DROP decisions to a log file sorted by timestamp. Developed as an Operating Systems course assignment.

## Overview

The project parallelizes a serial firewall (`serial.c`) using a producer-consumer architecture. The ring buffer is the shared data structure between the single producer and up to 32 consumer threads. All synchronization is done with mutexes and condition variables — no busy waiting.

## Architecture

```
[producer thread]
      |
      | ring_buffer_enqueue()
      v
[so_ring_buffer_t]  ←── mutex + cond vars
      |
      | ring_buffer_dequeue()
      v
[consumer thread 0]   [consumer thread 1]   ...   [consumer thread N]
      |                      |
      └──────────────────────┴──→ log file (sorted by timestamp)
```

## Components

### `ring_buffer.c` — Thread-Safe Circular Buffer

Fixed-capacity circular buffer with the following synchronization:
- `pthread_mutex_t mutex` — protects all accesses to shared state
- `pthread_cond_t are_date` — signals consumers when new data is available
- `pthread_cond_t are_loc` — signals producer when space becomes available
- `int stop` — set by `ring_buffer_stop()` to signal consumers to drain and exit

Key behaviors:
- `ring_buffer_enqueue()`: blocks if the buffer is full (waits on `are_loc`), writes data byte by byte with wrap-around, then signals `are_date`
- `ring_buffer_dequeue()`: blocks if buffer is empty and not stopped (waits on `are_date`); returns 0 when stopped and empty, causing consumer threads to exit
- Buffer capacity: `PKT_SZ * 1000` = 256 000 bytes (1000 packets)

### `producer.c` — Packet Publisher

Reads 256-byte packets from a binary input file and enqueues them into the ring buffer one by one. Calls `ring_buffer_stop()` after all packets have been enqueued to notify consumers.

### `consumer.c` — Packet Processor

Each consumer thread loops on `ring_buffer_dequeue()` until it returns 0. For each packet it calls `process_packet()` to get a PASS/DROP verdict, then `packet_hash()` for the packet hash, and writes the result to the output file.

### `packet.c` — Firewall Logic

`process_packet()` checks the packet's source IP against three allowed ranges. `packet_hash()` computes a deterministic hash over the full 256-byte packet using 50 iterations of a djb2-style hash.

### `firewall.c` — Entry Point

Initializes the ring buffer, spawns `num_consumers` consumer threads, calls `publish_data()` (producer, runs on the main thread), then `pthread_join`s all consumers before exiting.

## Log Format

Each line in the output file:
```
PASS|DROP <16-char hex hash> <timestamp>
```
Example:
```
PASS 0123456789abcdef 1714000000
DROP fedcba9876543210 1714000001
```

> **Note:** The current implementation does not guarantee timestamp-ordered output across concurrent consumers (Task 3 / sorted log was not fully implemented).

## Building & Running

```bash
make

# Parallel firewall
./firewall <input-file> <output-file> <num-consumers>

# Serial reference
./serial <input-file> <output-file>
```

## Testing

```bash
cd tests/
./grade.sh        # full grading
make check        # build and run all tests
make lint         # run checkpatch, cpplint, shellcheck
```

## Project Structure

```
src/
├── firewall.c        # Main entry point, thread management
├── ring_buffer.c/h   # Thread-safe circular buffer
├── producer.c/h      # Packet publisher
├── consumer.c/h      # Packet consumer / firewall logic
├── packet.c/h        # Packet hashing and filter rules
├── serial.c          # Serial reference implementation
└── Makefile
utils/
├── block_meta.h
└── ...
tests/
└── ...
```

## Grading Breakdown

| Task | Points | Description |
|---|---|---|
| Ring buffer + single consumer | 10 | Basic working implementation |
| Multi-consumer (unsorted) | 50 | Correct parallel processing |
| Multi-consumer (sorted by timestamp) | 30 | Log written in timestamp order during processing |
| Coding style | 10 | Linter compliance |
