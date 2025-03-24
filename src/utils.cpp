#include "utils.h"
#include <cstring>
#include "afc.h"
#include <fcntl.h>    // Para open()
#include <unistd.h>   // Para read() e close()

double randomMB_generator(int min, int max) {
    double value = min + (std::rand() % (max - min + 1));
    return value;
}

long readProcessStatus(pid_t pid, std::string option) {
    // mount the path
    std::ostringstream path;
    path << "/proc/" << getpid() << "/status";

    // get the status file
    std::ifstream file(path.str());
    if (!file) {
        std::cerr << "Erro ao abrir" << path.str() << "\n";
        return (long)0;
    }

    std::string line;
    long rss_anon = 0, rss_file = 0;
    // readubg the infos
    while (std::getline(file, line)) {
        if (line.find("VmSize:") != std::string::npos) {
            sscanf(line.c_str(), "VmSize: %ld kB", &rss_anon);
        }
        if (line.find("VmRSS:") != std::string::npos) {
            sscanf(line.c_str(), "VmRSS: %ld kB", &rss_file);
        }
    }

    if (option == "read") {
        return rss_file;
    } else if (option == "write") {
        return rss_anon;
    } else {
        return (long)0;
    }
}

void* allocateMemory(size_t size) {
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Falha na alocação de memória!" << std::endl;
        return nullptr;
    }
    return ptr;
}

void stressMemory(int process_id) {
    std::cout << "Init process " << process_id << " PID: " << getpid() << std::endl; 

    while (true) {
        long before_read = getMemoryUsageRead(getpid());
        long before_write = getMemoryUsageWrite(getpid());

        double alloc_size = (randomMB_generator(10, 100) * 1024 * 1024);
        void* data = allocateMemory(alloc_size); // 100 MB
        if (data == nullptr) continue;
        

        int fd = open("/dev/zero", O_RDONLY);
        if (!open) {
            std::cout << "...";
        }
        char buffer[4096];

        for (int i = 0; i < 100000; ++i) {
            read(fd, buffer, sizeof(buffer)); // Força leituras
        }
        close(fd);

        memset(data, 42, alloc_size);

        sleep(1);

        long after_read = getMemoryUsageRead(getpid());
        long after_write = getMemoryUsageWrite(getpid());
        
                std::cout << "Processo " << process_id << " (PID: " << getpid() << "):" << std::endl
                            << "  [READ USAGE]: " << after_read << " KB (delta: " << (after_read - before_read) << " KB)" << std::endl
                            << "  [WRITE USAGE]: " << after_write << " KB (delta: " << (after_write - before_write) << " KB)" << std::endl
                            << std::flush;
        
        munmap(data, (alloc_size));
        

        sleep(2);

    }
}

