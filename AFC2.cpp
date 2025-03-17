#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#define Kp 0.9 // proportional gain (ajustable)
#define max_limit 1000 
#define min_limit 100
#define adjust_limit_high 0.9
#define adjust_limit_lower 0.6

double budget = 500;



