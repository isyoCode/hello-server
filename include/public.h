#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>

#define LOG(str)                                                             \
  do {                                                                       \
    auto now = std::chrono::system_clock::to_time_t(                         \
        std::chrono::system_clock::now());                                   \
    std::cout << __FILE__ << ":" << __LINE__ << " ["                         \
              << std::put_time(std::localtime(&now), "%F %T") << "] " << str \
              << std::endl;                                                  \
  } while (0)