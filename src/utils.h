#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <iostream>

// gererate a random number of MB to allocator os free
double randomMB_generator(int min, int max);

// read the thread status
long readProcessStatus(pid_t tid, std::string option);

// memory stress
void stressMemory(int process_id);