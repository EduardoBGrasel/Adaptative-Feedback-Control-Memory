#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <sys/syscall.h>

#define Kp 0.6
#define max_limit 1000
#define min_limit 10
#define NUM_PROCESSES 4

double global_budget = 800;

struct ProcessMemory {
    double read_budget;
    double write_budget;
    double read_usage;
    double write_usage;
    pid_t thread_id;
    std::vector<std::vector<char>> memory_blocks;
};

std::vector<ProcessMemory> processes(NUM_PROCESSES, {200, 200, 0, 0, 0});

std::mutex budget_mutex;
std::condition_variable budget_cv;

int random_value(int min, int max) {
    return min + (std::rand() % (max - min + 1));
}

void MemoryOscillation(int pid) {
    processes[pid].thread_id = syscall(SYS_gettid);

    while (true) {
        int allocation_size = random_value(10, 120);
        int deallocation_size = random_value(10, allocation_size);

        {
            std::unique_lock<std::mutex> lock(budget_mutex);

            if (global_budget < allocation_size) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(random_value(500, 1500)));
                continue;
            }

            std::vector<char> block(allocation_size * 1024 * 1024, 1);
            processes[pid].memory_blocks.push_back(std::move(block));

            for (size_t i = 0; i < processes[pid].memory_blocks.back().size(); i += 4096) {
                processes[pid].memory_blocks.back()[i] = static_cast<char>(i % 256);
            }

            processes[pid].read_usage += allocation_size / 2.0;
            processes[pid].write_usage += allocation_size / 2.0;
            global_budget -= allocation_size;

            std::cout << "[THREAD " << processes[pid].thread_id << "] Alocado " << allocation_size << "MB | Read: "
                      << processes[pid].read_usage << "MB | Write: "
                      << processes[pid].write_usage << "MB | Global Budget: "
                      << global_budget << "MB\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(random_value(500, 1500)));

        {
            std::lock_guard<std::mutex> lock(budget_mutex);

            if (!processes[pid].memory_blocks.empty()) {
                processes[pid].memory_blocks.pop_back();
                processes[pid].read_usage -= deallocation_size / 2.0;
                processes[pid].write_usage -= deallocation_size / 2.0;
                global_budget += deallocation_size;

                if (processes[pid].read_usage < 0) processes[pid].read_usage = 0;
                if (processes[pid].write_usage < 0) processes[pid].write_usage = 0;
                if (global_budget > 800) global_budget = 800;

                std::cout << "[THREAD " << processes[pid].thread_id << "] Liberado " << deallocation_size << "MB | Read: "
                          << processes[pid].read_usage << "MB | Write: "
                          << processes[pid].write_usage << "MB | Global Budget: "
                          << global_budget << "MB\n";
            }
            budget_cv.notify_all();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(random_value(500, 1500)));
    }
}

void MemoryMeasurement(int pid) {
    while (true) {
        std::ifstream statusFile("/proc/self/status");
        if (!statusFile) {
            std::cerr << "Erro ao ler /proc/self/status\n";
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

            if (key == "VmRSS:") vmRSS = value / 1024.0;
            if (key == "VmData:") vmData = value / 1024.0;
        }

        {
            std::lock_guard<std::mutex> lock(budget_mutex);
            processes[pid].read_usage = vmRSS;
            processes[pid].write_usage = vmData;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void ReadMemoryAllocator(int pid) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(budget_mutex);

            double error = processes[pid].read_usage - processes[pid].read_budget;
            double adjust = Kp * error + random_value(-5, 5);

            processes[pid].read_budget += adjust;

            if (processes[pid].read_budget > max_limit) processes[pid].read_budget = max_limit;
            if (processes[pid].read_budget < min_limit) processes[pid].read_budget = min_limit;

            std::cout << "[THREAD " << processes[pid].thread_id << "][READ] Ajustado para " << processes[pid].read_budget << "MB\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void WriteMemoryAllocator(int pid) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(budget_mutex);

            double error = processes[pid].write_usage - processes[pid].write_budget;
            double adjust = Kp * error + random_value(-5, 5);

            processes[pid].write_budget += adjust;

            if (processes[pid].write_budget > max_limit) processes[pid].write_budget = max_limit;
            if (processes[pid].write_budget < min_limit) processes[pid].write_budget = min_limit;

            std::cout << "[THREAD " << processes[pid].thread_id << "][WRITE] Ajustado para " << processes[pid].write_budget << "MB\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    srand(time(0));
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        threads.emplace_back(MemoryMeasurement, i);
        threads.emplace_back(ReadMemoryAllocator, i);
        threads.emplace_back(WriteMemoryAllocator, i);
        threads.emplace_back(MemoryOscillation, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    return 0;
}
