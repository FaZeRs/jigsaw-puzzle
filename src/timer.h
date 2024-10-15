#pragma once

#include <chrono>

class Timer {
 private:
  using clock_t = std::chrono::high_resolution_clock;
  using millisecond_t = std::chrono::duration<double, std::milli>;
  std::chrono::time_point<clock_t> m_start_time;

 public:
  Timer() : m_start_time(clock_t::now()) {}

  void reset() { m_start_time = clock_t::now(); }

  double elapsedMs() const { return std::chrono::duration_cast<millisecond_t>(clock_t::now() - m_start_time).count(); }
};