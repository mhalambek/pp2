#pragma once

#include <chrono>

using namespace std::chrono;

struct Timer {

  high_resolution_clock::time_point start_time;
  high_resolution_clock::time_point end_time;

  Timer() {}

  void start()
  {
    start_time = high_resolution_clock::now();
  }

  long diffMilliseconds()
  {
    end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time).count();

    return duration;
  }
};
