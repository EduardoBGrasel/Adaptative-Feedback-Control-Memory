#include <iostream>
#include <ctime>
#include <fstream>

#define NUM_PROCESS 4
#define Kp 0.5


struct ProcessMemory {
    double read_budget;
    double write_budget;
    double read_usage;
    double write_usage;
};

// get the current read memory usage
double getMemoryUsageRead(int pid);
// get the current write memory usage
double getMemoryUsageWrite();
//
double readMemory();
double writeMemory();

void MemoryOcilation();

