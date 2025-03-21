#include "utils.h"

double randomMB_generator(int min, int max) {
    double value = min + (std::rand() % (max - min + 1));
    return value;
}
