#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>

#define Kp 0.5  // Controle adaptativo
#define max_limit 1000
#define min_limit 10
#define NUM_PROCESSES 4  // 4 processos estressando memória

double global_budget = 800;  // Orçamento total compartilhado

struct ProcessMemory {
    double read_budget;
    double write_budget;
    double read_usage;
    double write_usage;
};

std::vector<ProcessMemory> processes(NUM_PROCESSES, {200, 200, 0, 0});

std::mutex budget_mutex;
std::condition_variable budget_cv;

// Simula a alocação de memória para leitura e escrita
void MemoryOscillation(int pid) {
    std::vector<std::vector<int>> memory_blocks;
    
    while (true) {
        {
            std::unique_lock<std::mutex> lock(budget_mutex);

            // Espera até que o orçamento permita a alocação
            budget_cv.wait(lock, [&] {
                return (processes[pid].read_usage + 50 <= processes[pid].read_budget) &&
                       (processes[pid].write_usage + 50 <= processes[pid].write_budget) &&
                       (global_budget - 50 >= 0);
            });

            // Aloca memória
            memory_blocks.push_back(std::vector<int>(5000000, 1));
            processes[pid].read_usage += 25;
            processes[pid].write_usage += 25;
            global_budget -= 50;

            std::cout << "[PROC " << pid << "] Alocado 50MB (Read: " << processes[pid].read_usage 
                      << "MB, Write: " << processes[pid].write_usage << "MB)\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> lock(budget_mutex);
            if (!memory_blocks.empty()) {
                memory_blocks.pop_back();  // Libera memória
                processes[pid].read_usage -= 25;
                processes[pid].write_usage -= 25;
                global_budget += 50;

                std::cout << "[PROC " << pid << "] Liberado 50MB (Read: " << processes[pid].read_usage 
                          << "MB, Write: " << processes[pid].write_usage << "MB)\n";
            }

            budget_cv.notify_all();  // Avisa que memória foi liberada
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Mede o uso de memória
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

// Controla o orçamento de leitura com feedback
void ReadMemoryAllocator(int pid) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(budget_mutex);

            double error = processes[pid].read_usage - processes[pid].read_budget;
            double adjust = Kp * error;

            processes[pid].read_budget += adjust;

            if (processes[pid].read_budget > max_limit) processes[pid].read_budget = max_limit;
            if (processes[pid].read_budget < min_limit) processes[pid].read_budget = min_limit;

            std::cout << "[PROC " << pid << "][READ] Ajustado para " << processes[pid].read_budget << "MB\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// Controla o orçamento de escrita com feedback
void WriteMemoryAllocator(int pid) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(budget_mutex);

            double error = processes[pid].write_usage - processes[pid].write_budget;
            double adjust = Kp * error;

            processes[pid].write_budget += adjust;

            if (processes[pid].write_budget > max_limit) processes[pid].write_budget = max_limit;
            if (processes[pid].write_budget < min_limit) processes[pid].write_budget = min_limit;

            std::cout << "[PROC " << pid << "][WRITE] Ajustado para " << processes[pid].write_budget << "MB\n";
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
