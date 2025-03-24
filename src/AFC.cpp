#include "afc.h"
#include "utils.h"
#include <fstream>



double getMemoryUsageRead(pid_t pid) {
    std::string read = "read";
    long i = readProcessStatus(pid, read);
    return i;
}

double getMemoryUsageWrite(pid_t pid) {
    std::string write = "write";
    long i = readProcessStatus(pid, write);
    return i;
}

double readMemoryRegulation(double real_use) {
    
}
