#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <mutex>
#include <sstream>

#define Kp 0.5 // proportional gain (ajustable)
#define max_limit 1000 
#define min_limit 10

double read_budget = 200;
double write_budget = 200;

double shared_read_usage;
double shared_write_usage;

std::mutex read_mutex;
std::mutex write_mutex;

void MemoryOscillation() {
    std::vector<std::vector<int>> memory_blocks;
    while (true) {
        memory_blocks.push_back(std::vector<int>(100000000, 1));
        //std::cout << "Incrising memory usage\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // free memory
        if (!memory_blocks.empty()) {
            memory_blocks.pop_back();
            //std::cout << "decreasing memory\n";
        }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// simulates the real memory usage of a process

void MemoryMeasurement() {
    while (true) {
        std::ifstream statusFile("/proc/self/status");
        if (!statusFile) {
            std::cerr << "Error: cannot read /proc/self/status\n";
            continue;
        }

        std::string line;
        double vmRSS = 0.0, vmData = 0.0;

        while (std::getline(statusFile, line)) {
            std::istringstream iss(line);
            std::string key;
            double value;
            std::string unit;

            iss >> key >> value >> unit;

            if (key == "VmRSS:") {
                vmRSS = value / 1024.0;  // Convertendo KB para MB
                printf("vmRSS: %g\n", vmRSS);
            }
            if (key == "VmData:") {
                vmData = value / 1024.0; // Convertendo KB para MB
                printf("vmDATA: %g\n", vmData);
            }
        }

        double read_mem = vmRSS;  // Considera toda a memória residente como leitura
        double write_mem = vmData; // Considera VmData como escrita

        {
            std::lock_guard<std::mutex> guard(read_mutex);
            shared_read_usage = read_mem;
        }

        {
            std::lock_guard<std::mutex> guard(write_mutex);
            shared_write_usage = write_mem;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// void MemoryMeasurement() {
//     while (true) {
//         std::ifstream statm("/proc/self/statm");
//         if (!statm) {
//             std::cerr << "error, cannot read /proc/self/statm\n";
//             continue;
//         }
//         long pages, resident, shared, text, lib, data, dirty;
//         statm >> pages >> resident >> shared >> text >> lib >> data >> dirty;
    
//         long page_size = sysconf(_SC_PAGE_SIZE) / 1024; // page size in KB
//         double mem_usage = (pages * page_size) / 1024.0; // convertion to MB
//         double write_mem = ((resident - shared) * page_size) / 1024.0;
//         double read_mem = (mem_usage - write_mem);
        
//         {
//             std::lock_guard<std::mutex> guard(read_mutex);
//             shared_read_usage = read_mem;
//         }

//         {
//             std::lock_guard<std::mutex> guard(write_mutex);
//             shared_write_usage = write_mem;
//         }
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//     }
//     //return 400.0 + (rand() % 300 - 150); // return between 250MB to 550MB
//     //return 600;
// }

void ReadMemoryAllocator() {
    while (true) {
        double current_read_usage;

        {
            std::lock_guard<std::mutex> guard(read_mutex);
            current_read_usage = shared_read_usage;
        }

        std::cout << "[READ] Current usage: " << current_read_usage << "MB\n";
        double error = current_read_usage - read_budget;
        double adjust = Kp * error;

        {
            std::lock_guard<std::mutex> guard(read_mutex);
            read_budget += adjust;

            if (read_budget > max_limit) read_budget = max_limit;
            if (read_budget < min_limit) read_budget = min_limit; 
            std::cout << "[READ] New budget: " << read_budget << "MB\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void WriteMemoryAllocator() {
    while (true) {
        double current_write_usage;

        {
            std::lock_guard<std::mutex> guard(write_mutex);
            current_write_usage = shared_write_usage;
        }

        std::cout << "[WRITE] Current usage: " << current_write_usage << "MB\n";
        double error = current_write_usage - write_budget;
        double adjust = Kp * error;

        {
            std::lock_guard<std::mutex> guard(write_mutex);
            write_budget += adjust;
            if (write_budget > max_limit) write_budget = max_limit;
            if (write_budget < min_limit) write_budget = min_limit;

            std::cout << "[WRITE] New budget: " << write_budget << "MB\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// // Simulates the new memory needed
// void MemoryAlloc(double new_budget) {
//     std::cout << "New memory budget: " << new_budget << "MB\n";
// }

// void AdaptativeMemoryControl() {
//     while (true) {
//         double current_usage = MemoryMeasurement();
//         std::cout << "current usage: " << current_usage << std::endl;
//         double error = current_usage - budget;

//         double adjust = Kp * error;
//         //std::cout << "adjust: " << current_usage << std::endl;
        
//         budget += adjust;

//         if (budget > max_limit) budget = max_limit;
//         if (budget < min_limit) budget = min_limit;

//         // applies the changes
//         MemoryAlloc(budget);

//         std::this_thread::sleep_for(std::chrono::seconds(2));
//     }
// }

int main() {
    srand(time(0));

    std::thread readerThread(MemoryMeasurement);
    std::thread readAllocatorThread(ReadMemoryAllocator);
    std::thread writeAllocatorThread(WriteMemoryAllocator);
    std::thread oscillationThread(MemoryOscillation);

    readerThread.join();
    readAllocatorThread.join();
    writeAllocatorThread.join();
    oscillationThread.join();

    return 0;
}