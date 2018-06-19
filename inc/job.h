//
// Created by Andrei - Doru Pata on 19/06/18.
//
#pragma once

#include <functional>

namespace cronjob {

struct Job {  
  Job() = default;
  
  /**
   * @brief Job's constructor.
   * @tparam _Callable - A Callable type.
   * @tparam _Args     - Callable's parameter-pack.
   * @param reiterable - true - The job will be rescheduled.
   *                   - false - One-shoot job.
   * @param interval   - Job's reschedule interval.
   * @param function   - Job to execute.
   * @param args       - Job's parameters.
   */
  template <typename _Callable, typename... _Args>
  explicit Job(bool reiterable, int interval,
               _Callable&& function, _Args&& ... args) :
      reiterable_{ reiterable },
      interval_{ interval },
      function_{ std::bind(std::forward<_Callable>(function),
                           std::forward<_Args>(args)...) } {
    if (interval_ < 1) {
      throw std::runtime_error("CronJob's interval is less than 1 second");
    }
  }
 
 private:
  friend class Scheduler;
  
  bool reiterable_;
  int interval_;
  std::function<void()> function_;
};
} // namespace cronjob