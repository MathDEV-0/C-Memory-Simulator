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
char virtualMemory[MAX_PROC][N_PAGES][PAGES_SIZE];

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
int _MMUMap(int pid, int virtualAdress);
void showFrames();
void generateRandomAccesses(int n);
void showMemoryUsagePerProcess();
void showMainMemory();
void showVirtualMemory(int pid);
void pageToDisk(int pid, int page, char *data);
void readPageFromDisk(int pid, int page, char *buffer);

int main(int argc, char* argv[]){
    init();

    int addr1 = 1000;
    int addr2 = 9000;
    int addr3 = 20000; 
    int addr4 = 20001; 
    int addr5 = 20004; 
    int addr6 = 20003; 

    _MMUMap(0, addr1);
    _MMUMap(0, addr2);
    _MMUMap(1, addr1);
    _MMUMap(0, addr3);
    _MMUMap(2, addr4);
    _MMUMap(3, addr5);
    _MMUMap(3, addr6);
    _MMUMap(0, 30000);
    _MMUMap(0, 40000); 
    _MMUMap(0, 50000);
    _MMUMap(1, 60000);
    _MMUMap(1, 70000);
    _MMUMap(1, 800000);
    _MMUMap(0, 800002);
    _MMUMap(0, 1048575);
    _MMUMap(0, 900000000);

    _MMUMap(0, 0x000003E8);  
    _MMUMap(0, 0x00002328);   
    _MMUMap(1, 0x000003E8);   
    _MMUMap(0, 0x00004E20);  
    _MMUMap(0, 0x00007530); 
    _MMUMap(0, 0x00009C40);   
    _MMUMap(0, 0x0000C350); 
    _MMUMap(1, 0x0000EA60);   
    _MMUMap(1, 0x00011170);   
    _MMUMap(1, 0x000C3500); 
    _MMUMap(0, 0x000C3502);  
    _MMUMap(0, 0x000FFFFF);  
    _MMUMap(1, 0x000FFFFA);  
    _MMUMap(0, 0x35A4E900);   
    // generateRandomAccesses(10);

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

    for (int i = 0; i < MAX_PROC; i++) {
        for (int j = 0; j < N_PAGES; j++) {
            for (int k = 0; k < PAGES_SIZE; k++) {
                virtualMemory[i][j][k] = EMPTY;
            }
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
    
    char buffer[PAGES_SIZE];

    readPageFromDisk(pid, page, buffer);

    int empty = 1;
    for (int i = 0; i < PAGES_SIZE; i++) {
        if (buffer[i] != 0) {
            empty = 0;
            break;
        }
    }

    if (empty) {
        for (int i = 0; i < PAGES_SIZE; i++) {
            buffer[i] = (page + i + pid) % 256;
        }
        pageToDisk(pid, page, buffer);
    }

    memcpy(mainMemory[frame], virtualMemory[pid][page], PAGES_SIZE); //Como se fosse copiar a página do disco pra RAM

    enqueue(&fifoStruct, frame);
    printQueue(&fifoStruct);
    printf("[PID %d] DONE | Page %d mapped to Frame %d\n", pid, page, frame);
}

int _MMUMap(int pid, int virtualAddress) {
    if (pid >= MAX_PROC) {
        printf("\nPID %d is invalid(PID greater or equal the number of processess [%d])\n",pid,MAX_PROC);
        return -1;
    }

    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        printf("\nAddress  0x%X is invalid (address is greater or equal the virtual memory size [%d/0x%X])\n",virtualAddress,VIRTUAL_MEMORY_SIZE,VIRTUAL_MEMORY_SIZE);
        return -1;
    }

    int page = virtualAddress / PAGES_SIZE;
    int offset = virtualAddress % PAGES_SIZE;

    printf("\n[PID %d] Request | Virtual Address:  0x%X (Page: %d, Offset: %d)\n",pid, virtualAddress, page, offset);

    if (pageTable[pid][page].inRAM) {
        int frame = pageTable[pid][page].frame;
        int physicalAddr = frame * PAGES_SIZE + offset;

        printf("[PID %d] HIT | Page %d is in Frame %d\n", pid, page, frame);
        printf("[PID %d] Translated | Physical Address:  0x%X\n", pid, physicalAddr);
        char value = mainMemory[frame][offset];
        printf("[PID %d] VALUE at address: %d\n", pid, (unsigned char)value);

        hits++;
        return physicalAddr;
    } else {
        printf("[PID %d] PAGE FAULT | Page %d not in memory\n", pid, page);
        handlePageFault(pid, page);
        faults++;

        return _MMUMap(pid, virtualAddress);
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
        _MMUMap(pid, address);
        sleep(1);
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
                printf("%d ", (unsigned char)virtualMemory[pid][i][j]);
            }
        } else {
            printf("...");
        }

        printf("\n");
    }
}

void pageToDisk(int pid, int page, char *data) {
    char filename[50];
    sprintf(filename, "logs/virtual_mem_pid_%d.bin", pid);

    FILE *f = fopen(filename, "r+b");
    if (!f) {
        f = fopen(filename, "w+b");
    }

    fseek(f, page * PAGES_SIZE, SEEK_SET);
    fwrite(data, sizeof(char), PAGES_SIZE, f);

    fclose(f);
}

void readPageFromDisk(int pid, int page, char *buffer) {
    char filename[50];
    sprintf(filename, "logs/virtual_mem_pid_%d.bin", pid);

    FILE *f = fopen(filename, "rb");

    if (!f) {
        memset(buffer, 0, PAGES_SIZE);
        return;
    }

    fseek(f, page * PAGES_SIZE, SEEK_SET);
    fread(buffer, sizeof(char), PAGES_SIZE, f);

    fclose(f);
}