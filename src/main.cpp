#include "afc.h"
#include "utils.h"
#include <cstring>
#include <sys/wait.h>


int main() {
    
    std::vector<pid_t> pids;
    for (int i = 0; i < NUM_PROCESS; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            srand(getpid());
            stressMemory(i);
            exit(0);
        } else if (pid > 0) {
            pids.push_back(pid);
            std::cout << "Processo filho: " << i+1 << " com pid " << pid << " criado" << std::endl;
        } else {
            std::cout << "erro";
        }
    }

    for (pid_t pid : pids) {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "\n\n\nProcesso com o PID " << pid << " terminou com status " << status << "\n\n\n";
    }

    return 0;
}