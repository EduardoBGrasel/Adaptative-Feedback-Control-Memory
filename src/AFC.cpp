#include "afc.h"
#include "utils.h"
#include <fstream>


double getMemoryUsageRead(int pid) {
    std::ifstream statusFile("/proc/self/status");
    if (!statusFile) {
        std::cerr << "Cannot read /proc/self/status\n";
    }
    else {
        std::cout << "feito";
    }
    return 2.3;
}
