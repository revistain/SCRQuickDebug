#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <vector>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <iostream>
#include <windows.h>
#include <ctime>
#include <atomic>
#include <chrono>
#include "variable.h"

void ensureEndTimestampThread();
void analyzeTimeStamp(std::unique_ptr<Variables>& var_ptr);

void startTimeStamp(std::unique_ptr<Variables>& var_ptr);
void endTimeStamp();

#endif