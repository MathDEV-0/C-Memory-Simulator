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
9.  [Execution Scenarios](#execution-scenarios)
10.  [License](#license)

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

## Execution Scenarios

The simulator handles the following execution flows:

### 1\. Hit (page already in RAM)
```
[PID 0] Request | Virtual Address:  0x3E8 (Page: 0, Offset: 1000)\
[PID 0] HIT | Page 0 is in Frame 0\
[PID 0] Translated | Physical Address:  0x3E8\
[PID 0] VALUE at address: 0
```
* * * * *

### 2\. Page Fault (first access → load → hit)
```
[PID 0] Request | Virtual Address:  0x3E8 (Page: 0, Offset: 1000)\
[PID 0] PAGE FAULT | Page 0 not in memory\
[PID 0] Loading Page 0 into Frame 0...\
Current Queue: 0\
[PID 0] DONE | Page 0 mapped to Frame 0

[PID 0] Request | Virtual Address:  0x3E8 (Page: 0, Offset: 1000)\
[PID 0] HIT | Page 0 is in Frame 0
```
* * * * *

### 3\. Page Fault with FIFO Replacement (RAM full)
```
[PID 1] Request | Virtual Address:  0x11170 (Page: 8, Offset: 4464)\
[PID 1] PAGE FAULT | Page 8 not in memory\
Current Queue: 1 2 3 4 5 6 7\
[REPLACE] Removing Page 0 (PID 0) from Frame 0\
[PID 1] Loading Page 8 into Frame 0...\
Current Queue: 1 2 3 4 5 6 7 0\
[PID 1] DONE | Page 8 mapped to Frame 0
```
* * * * *

### 4\. Re-access after replacement
```
[PID 1] Request | Virtual Address:  0x11170 (Page: 8, Offset: 4464)\
[PID 1] HIT | Page 8 is in Frame 0\
[PID 1] Translated | Physical Address:  0x1170
```
* * * * *

### 5\. Invalid PID
```
PID 2 is invalid(PID greater or equal the number of processess [2])
```
* * * * *

### 6\. Invalid Address (out of bounds)
```
Address  0x35A4E900 is invalid (address is greater or equal the virtual memory size [1048576/0x100000])
```
* * * * *

### 7\. Same virtual page, different processes
```
[PID 1] Loading Page 97 into Frame 5...\
[PID 0] Loading Page 97 into Frame 6...
```

Different PIDs → same page number → different frames/data.
## License

This project is intended for educational purposes. You are free to use and modify it as needed.
