#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

#define KB 1024
#define MB (1024 * KB)
#define MAIN_MEMORY_SIZE (64*KB)
#define VIRTUAL_MEMORY_SIZE (1 * MB)
#define PAGES_SIZE (8 * KB)
#define N_FRAMES (MAIN_MEMORY_SIZE / PAGES_SIZE)
#define N_PAGES (VIRTUAL_MEMORY_SIZE/PAGES_SIZE)
#define MAX_PROC 2
#define EMPTY 0
#define OCCUPIED 1

typedef struct{
    int inRAM;
    int frame;
}Page;

//Memory collections
//PAGE REPLACEMENT STRUCT
Queue fifoStruct;

char mainMemory[N_FRAMES][PAGES_SIZE];
char virtualMemory[N_PAGES][PAGES_SIZE];

Page pageTable[MAX_PROC][N_PAGES];

int frameToPage[N_FRAMES];
int frameToPid[N_FRAMES];

int frameUsed[N_FRAMES];

//Global Vars
int hits = 0;
int faults = 0;


//func
void init();
int findFrame();
void handlePageFault(int pid, int page);
int toMMU(int pid, int virtualAdress);
void showFrames();
void generateRandomAccesses(int n);
void showMemoryUsagePerProcess();
void showMainMemory();
void showVirtualMemory(int pid);

int main(int argc, char* argv[]){
    init();

    int addr1 = 1000;
    int addr2 = 9000;
    int addr3 = 20000; 
    int addr4 = 20001; 
    int addr5 = 20004; 
    int addr6 = 20003; 

    toMMU(0, addr1);
    toMMU(0, addr2);
    toMMU(1, addr1);
    toMMU(0, addr3);
    toMMU(2, addr4);
    toMMU(3, addr5);
    toMMU(3, addr6);
    toMMU(0, 30000);
    toMMU(0, 40000); 
    toMMU(0, 50000);
    toMMU(1, 60000);
    toMMU(1, 70000);
    toMMU(1, 800000);
    toMMU(0, 800002);
    toMMU(0, 1048575);
    toMMU(0, 900000000);

    // generateRandomAccesses(50);

    printf("\nHits: %d\n", hits);
    printf("\nFaults: %d\n", faults);

    showFrames();
    showMemoryUsagePerProcess();

    printf("\n==== STATISTICS ====\n");
    printf("Frames: %d\n", N_FRAMES);
    printf("Pages: %d\n", N_PAGES);
    printf("Hits: %d\n", hits);
    printf("Faults: %d\n", faults);
    printf("=================\n");
    showMainMemory();

    for(int i = 0; i < MAX_PROC;i++){
        showVirtualMemory(i);
    }

    return 0;
}


void init() {
    initializeQueue(&fifoStruct);
    // printQueue(&fifoStruct);

    for (int i = 0; i < MAX_PROC; i++) {
        for (int j = 0; j < N_PAGES; j++) {
            pageTable[i][j].inRAM = 0;
            pageTable[i][j].frame = -1;
        }
    }

    for (int i = 0; i < N_FRAMES; i++) {
        frameUsed[i] = 0;
        frameToPage[i] = -1;
        frameToPid[i] = -1;
    }

    for (int i = 0; i < N_PAGES; i++) {
        for (int j = 0; j < PAGES_SIZE; j++) {
            virtualMemory[i][j] = EMPTY; 
        }
    }
}
int findFrame() {
    for (int i = 0; i < N_FRAMES; i++) {
        if (frameUsed[i] == 0)
            return i;
    }
    return -1;
}
void handlePageFault(int pid, int page) {
    int frame = findFrame();

    if (frame == -1) {
        frame = dequeue(&fifoStruct);
        printQueue(&fifoStruct);

        int oldPage = frameToPage[frame];
        int oldPid  = frameToPid[frame];

        printf("[REPLACE] Removing Page %d (PID %d) from Frame %d\n", oldPage, oldPid, frame);
        pageTable[oldPid][oldPage].inRAM = 0;
        pageTable[oldPid][oldPage].frame = -1;
    }

    printf("[PID %d] Loading Page %d into Frame %d...\n", pid, page, frame);

    frameUsed[frame] = 1;

    pageTable[pid][page].inRAM = 1;
    pageTable[pid][page].frame = frame;

    frameToPage[frame] = page;
    frameToPid[frame] = pid;
    
    memset(virtualMemory[page], pid + 1, PAGES_SIZE);
    memcpy(mainMemory[frame], virtualMemory[page], PAGES_SIZE); //Como se fosse copiar a página do disco pra RAM

    enqueue(&fifoStruct, frame);
    printQueue(&fifoStruct);
    printf("[PID %d] DONE | Page %d mapped to Frame %d\n", pid, page, frame);
}

int toMMU(int pid, int virtualAddress) {
    if (pid >= MAX_PROC) {
        printf("\nPID %d is in RAM(PID greater or equal the number of processess [%d])\n",pid,MAX_PROC);
        return -1;
    }

    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        printf("\nAddress %d is in RAM (address is greater or equal the virtual memory size [%d])\n",virtualAddress,VIRTUAL_MEMORY_SIZE);
        return -1;
    }

    int page = virtualAddress / PAGES_SIZE;
    int offset = virtualAddress % PAGES_SIZE;

    printf("\n[PID %d] Request | Virtual Address: %d (Page: %d, Offset: %d)\n",
           pid, virtualAddress, page, offset);

    if (pageTable[pid][page].inRAM) {
        int frame = pageTable[pid][page].frame;
        int physicalAddr = frame * PAGES_SIZE + offset;

        printf("[PID %d] HIT | Page %d is in Frame %d\n", pid, page, frame);
        printf("[PID %d] Translated | Physical Address: %d\n", pid, physicalAddr);
        hits++;
        return physicalAddr;
    } else {
        printf("[PID %d] PAGE FAULT | Page %d not in memory\n", pid, page);
        handlePageFault(pid, page);
        faults++;

        return toMMU(pid, virtualAddress);
    }
}

void showFrames() {
    printf("\nFrames:\n");
    for (int i = 0; i < N_FRAMES; i++) {
        printf("Frame %d | PID %d | Page %d\n",
               i, frameToPid[i], frameToPage[i]);
    }
}

void generateRandomAccesses(int n) {
    for (int i = 0; i < n; i++) {
        int pid = rand() % MAX_PROC;
        int address = rand() % VIRTUAL_MEMORY_SIZE;

        printf("\nREQUEST [%d]:\n", i + 1);
        toMMU(pid, address);
        sleep(0.1);
    }
}

void showMemoryUsagePerProcess() {
    int usage[MAX_PROC];

    for (int i = 0; i < MAX_PROC; i++) {
        usage[i] = 0;
    }

    for (int i = 0; i < N_FRAMES; i++) {
        if (frameToPid[i] != -1) {
            usage[frameToPid[i]]++;
        }
    }

    printf("\n=== MEMORY USAGE PER PROCESS ===\n");
    for (int i = 0; i < MAX_PROC; i++) {
        int bytes = usage[i] * PAGES_SIZE;
        printf("PID [%d] | Frames: %d | Memory: %d bytes (%.2f KB)\n",
               i, usage[i], bytes, bytes / 1024.0);
    }
}

void showMainMemory() {
    printf("\n=== MAIN MEMORY (RAM) ===\n");

    for (int i = 0; i < N_FRAMES; i++) {
        printf("Frame %d | PID %d | Page %d | Data preview: ",
               i, frameToPid[i], frameToPage[i]);

        if (frameUsed[i]) {
            for (int j = 0; j < 8; j++) {
                printf("%d ", (unsigned char)mainMemory[i][j]);
            }
        } else {
            printf("EMPTY");
        }

        printf("\n");
    }
}

void showVirtualMemory(int pid) {
    printf("\n=== VIRTUAL MEMORY (PID %d) ===\n", pid);

    for (int i = 0; i < N_PAGES; i++) {
        if(pageTable[pid][i].inRAM == 0){
            continue;
        }
        printf("Page %d | In RAM: %d | Frame: %d | Data preview: ",
               i,
               pageTable[pid][i].inRAM,
               pageTable[pid][i].frame);

        if (i < 128) {
            for (int j = 0; j < 8; j++) {
                printf("%d ", (unsigned char)virtualMemory[i][j]);
            }
        } else {
            printf("...");
        }

        printf("\n");
    }
}