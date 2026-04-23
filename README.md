# Virtual Memory Simulator

A C program that simulates a virtual memory management system with paging, page tables, and FIFO page replacement. Built to demonstrate how an MMU (Memory Management Unit) translates virtual addresses into physical addresses.

# Table of Contents

---

1.  [Overview](#overview)
2.  [Architecture](#architecture)
    - [Memory Model](#memory-model)
    - [Address Translation](#address-translation)
    - [Page Replacement](#page-replacement)
3.  [Components](#components)
4.  [Build Instructions](#build-instructions)
    - [Prerequisites](#prerequisites)
    - [Compilation](#compilation)
5.  [Usage](#usage)
6.  [Simulation Details](#simulation-details)
7.  [File Descriptions](#file-descriptions)
8.  [Disk Simulation](#disk-simulation)
9.  [License](#license)

---

## Overview

In operating systems, virtual memory allows processes to use more memory than physically available by abstracting memory into pages. The Memory Management Unit (MMU) is responsible for translating virtual addresses into physical addresses using page tables.

This project is a small educational simulator that implements:

- Virtual-to-physical address translation
- Page tables per process
- Page faults and handling
- FIFO (First-In-First-Out) page replacement
- Basic statistics (hits and faults)

The simulator mimics how an OS loads pages into RAM and replaces them when memory is full.

---

## Architecture

### Memory Model

The system is divided into:

- **Main Memory (RAM)**\
  Fixed-size frames that store pages currently in use.
- **Virtual Memory**\
  Larger memory space divided into pages.
- **Page Table (per process)**\
  Keeps track of:
  - Whether a page is in RAM (`inRAM`)
  - Which frame it occupies (`frame`)

Key constants:

- `MAIN_MEMORY_SIZE = 64 KB`
- `VIRTUAL_MEMORY_SIZE = 1 MB`
- `PAGE_SIZE = 8 KB`
- `N_FRAMES = MAIN_MEMORY_SIZE / PAGE_SIZE`
- `N_PAGES = VIRTUAL_MEMORY_SIZE / PAGE_SIZE`

---

### Address Translation

The MMU simulation is handled by the function:

toMMU(int pid, int virtualAddress)

Steps:

1.  Split virtual address into:
    - **Page number**
    - **Offset**
2.  Check page table:
    - If present → **HIT**
    - If not → **PAGE FAULT**
3.  If HIT:
    - Translate to physical address:

      physical = frame \* PAGE_SIZE + offset

4.  If FAULT:
    - Load page into memory
    - Retry translation

---

### Page Replacement

When RAM is full:

- The oldest loaded frame is removed using a **FIFO queue**
- The corresponding page table entry is invalidated
- The new page is loaded into the freed frame

This is handled by:

```
handlePageFault(int pid, int page)
```

Data structures used:

- Queue (`fifoStruct`) for FIFO replacement
- Arrays mapping:
  - `frame → page`
  - `frame → pid`

---

## Components

- `main.c` -- Main simulator implementation (MMU, paging, page faults, FIFO)
- `queue.h` -- Queue used to implement FIFO page replacement

- `mainMemory` -- Simulated physical memory (RAM), organized in frames
- `virtualMemory` -- Per-process virtual memory (used to initialize page data)
- `pageTable` -- Per-process page table mapping pages to frames

- `frameToPage` -- Maps each frame to the page currently stored
- `frameToPid` -- Maps each frame to the owning process (PID)
- `frameUsed` -- Indicates whether a frame is free or occupied

- `fifoStruct` -- Queue tracking load order for FIFO replacement

---

## Build Instructions

### Prerequisites

- GCC or any C compiler
- POSIX-compatible system (Linux/macOS recommended)

---

### Compilation
```
gcc main.c -o vm_simulator
```
Make sure `queue.h` is in the same directory.

---

## Usage

Run the program:

```
./vm_simulator
```

The program will:

- Execute a sequence of predefined memory accesses
- Print:
  - Address translations
  - Page faults
  - Frame allocations
  - Replacement operations

Optional:

Enable random accesses by uncommenting:

```
generateRandomAccesses(5);
```

---

## Simulation Details

### Example Behavior

- A process requests a virtual address
- If the page is not in RAM:
  - A **page fault** occurs
  - The page is loaded into a free frame or replaces another
- If already loaded:
  - A **hit** occurs

### Statistics Collected

- `hits` → successful accesses without fault
- `faults` → number of page faults

Final output includes:

- Frame state
- Memory usage per process
- Total statistics

---

## File Descriptions

| File              | Description                               |
| ----------------- | ----------------------------------------- |
| `main.c`          | Core simulation logic (MMU, paging, FIFO) |
| `queue.h`         | Queue implementation for page replacement |
| `output (stdout)` | Console logs showing simulation steps     |
| `logs/`           | Stores binary files that simulate disk |

---

## Disk Simulation

In this simulator, the disk is represented by binary files, one per process:
```
logs/virtual_mem*pid*<pid>.bin
```
Each file stores the process virtual memory as fixed-size pages:

- Page size: 8 KB
- Offset in file: page \* PAGE_SIZE

### Behavior

- On a page fault:
  - The system attempts to read the page from disk
  - If the page does not exist:
    - It is generated using a deterministic function
    - Then written to disk
  - The page is loaded into RAM

### Data Generation

Page contents are generated as:
```
(page + offset + pid) % 256
```
This ensures:

- Different pages have different data
- Different processes have different data
- Deterministic and reproducible behavior

## License

This project is intended for educational purposes. You are free to use and modify it as needed.
