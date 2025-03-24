#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <thread>
#include <sys/mman.h>
#include <unistd.h>

#define NUM_PROCESS 4
#define Kp 0.5


struct ProcessMemory {
    double read_budget;
    double write_budget;
    double read_usage;
    double write_usage;
};

// get the current read memory usage
double getMemoryUsageRead(pid_t pid);
// get the current write memory usage
double getMemoryUsageWrite(pid_t pid);

// regula o budget da memoria para leitura
double readMemoryRegulation();
// regula o budget para escrita
double writeMemoryRegularion();

void MemoryOcilation();

