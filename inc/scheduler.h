//
// Created by Andrei - Doru Pata on 16/06/18.
//
#pragma once

#include <iostream>
#include <mutex>
#include <list>
#include <memory>
#include <map>

#include "job.h"

namespace cronjob {

class Scheduler {
 public:
  Scheduler() = default;
  
  Scheduler(const Scheduler&) = delete;
  
  Scheduler(Scheduler&&) noexcept = delete;
  
  Scheduler& operator=(const Scheduler&) = delete;
  
  Scheduler& operator=(Scheduler&&) noexcept = delete;
  
  /**
   * @brief Update @current_time_.
   * if (@time == current_time_) => nothing will happen.
   * if (@time < current_time)   => no job will run, but all reiterable jobs
   *                                will be rescheduled from that moment.
   * if (@time > @current_time_) => all jobs, that should been triggered in this
   *                                period, will run just once and will be
   *                                rescheduled if reiterable.
   * @param time
   */
  void OnNewTime(const timeval& time);
  
  /**
   * @brief Add a new job to the @Scheduler.
   * @tparam _Callable - A Callable type.
   * @tparam _Args     - Callable's parameter-pack.
   * @param reiterable - true - The job will be rescheduled.
   *                   - false - One-shoot job.
   * @param interval   - Job's reschedule interval.
   * @param function   - Job to execute.
   * @param args       - Job's parameters.
   * @return
   */
  template <typename _Callable, typename... _Args>
  std::shared_ptr<Job> Add(bool reiterable, int interval,
                           _Callable&& function, _Args&& ... args);
  
  /**
   * @brief Remove a job from @Scheduler.
   * @param job - Job to be removed.
   */
  void Remove(std::shared_ptr<Job>& job);
 
 private:
  /**
   * @brief Reschedule all reiterable jobs in case of updating the
   * @current_time_ with a past time.
   * @note Get lock before calling this method.
   */
  void PastReschedule();
  
  /**
   * @brief Reschedule all reiterable jobs in case of updating the
   * @current_time_ with a future time.
   * @note Get lock before calling this method.
   */
  void FutureReschedule(const std::list<std::shared_ptr<Job>>& jobs_to_run);
  
  /**
   * @brief Get all jobs that needs to be executed.
   * @param jobs_to_run - All jobs that needs to be executed.
   * @note Get lock before calling this method.
   */
  void GetJobsToRun(std::list<std::shared_ptr<Job>>& jobs_to_run) const;
  
  std::mutex lock;
  unsigned int current_time_ = 0;
  // Maps moments of time to execute jobs with the corresponding jobs.
  std::multimap<int, std::shared_ptr<Job>> jobs{};
};

template <typename _Callable, typename... _Args>
std::shared_ptr<Job> Scheduler::Add(bool reiterable, int interval,
                                    _Callable&& function,
                                    _Args&& ... args) {
  auto job_ptr =
      std::make_shared<Job>(reiterable, interval,
                            std::bind(std::forward<_Callable>(function),
                                      std::forward<_Args>(args)...));
  
  std::lock_guard<std::mutex> lg(lock);
  
  jobs.emplace(current_time_ + job_ptr->interval_, job_ptr);
  return std::move(job_ptr);
}
} // namespace cronjob