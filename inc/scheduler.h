//
// Created by Andrei - Doru Pata on 16/06/18.
//
#pragma once

#include <iostream>
#include <mutex>
#include <list>
#include <memory>
#include <map>
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
   * @param args       - Job's arguments.
   */
  template <typename _Callable, typename... _Args>
  explicit Job(bool reiterable, int interval,
               _Callable&& function, _Args&& ... args);
 
 private:
  friend class Scheduler;
  
  bool reiterable_ = false;
  int interval_ = 1;
  std::function<void()> function_;
};

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
   * @brief Add a new repetitive job to the @Scheduler.
   * @tparam _Callable - A Callable type.
   * @tparam _Args     - Callable's parameter-pack.
   * @param interval   - Job's reschedule interval.
   * @param function   - Job to execute.
   * @param args       - Job's arguments.
   * @return
   */
  template <typename _Callable, typename... _Args>
  std::shared_ptr<Job> Run(int interval,
                           _Callable&& function,
                           _Args&& ... args);
  
  /**
 * @brief Add a new one-shot job to the @Scheduler.
 * @tparam _Callable - A Callable type.
 * @tparam _Args     - Callable's parameter-pack.
 * @param interval   - After how many seconds the job will run once.
 * @param function   - Job to execute.
 * @param args       - Job's arguments.
 * @return
 */
  template <typename _Callable, typename... _Args>
  std::shared_ptr<Job> RunOnce(int interval,
                               _Callable&& function,
                               _Args&& ... args);
  
  /**
   * @brief Remove a job from @Scheduler and resets the shared_ptr.
   * @param job - Job to be removed.
   * @note - Nothing is happening in case of double job removing.
   */
  void Remove(std::shared_ptr<Job>& job);
 
 private:
  /**
  * @brief Add a new job to the @Scheduler.
  * @tparam _Callable - A Callable type.
  * @tparam _Args     - Callable's parameter-pack.
  * @param reiterable - true - The job will be rescheduled.
  *                   - false - One-shoot job.
  * @param interval   - After how many seconds the job will run once.
  * @param function   - Job to execute.
  * @param args       - Job's arguments.
  * @return
  */
  template <typename _Callable, typename... _Args>
  std::shared_ptr<Job> Add(bool reiterable, int interval,
                           _Callable&& function,
                           _Args&& ... args);
  
  /**
   * @brief Reschedule all reiterable jobs in case of updating the
   * @current_time_ with a past time.
   * @note - Get lock before calling this method.
   */
  void PastReschedule();
  
  /**
   * @brief Reschedule all reiterable jobs in case of updating the
   * @current_time_ with a future time.
   * @param jobs_to_run - All jobs that needs to be rescheduled.
   * @note - Get lock before calling this method.
   */
  void FutureReschedule(const std::list<std::shared_ptr<Job>>& jobs_to_run);
  
  /**
   * @brief Get all jobs that needs to be executed.
   * @param jobs_to_run - All jobs that needs to be executed.
   * @note - Get lock before calling this method.
   */
  void GetJobsToRun(std::list<std::shared_ptr<Job>>& jobs_to_run) const;
  
  std::mutex lock_;
  long current_time_ = 0;
  // Maps moments of time (to execute jobs_) with the corresponding jobs_.
  std::multimap<long, std::shared_ptr<Job>> jobs_{};
};

template <typename _Callable, typename... _Args>
Job::Job(bool reiterable, int interval,
         _Callable&& function, _Args&& ... args) :
    reiterable_{reiterable},
    interval_{interval},
    function_{std::bind(std::forward<_Callable>(function),
                        std::forward<_Args>(args)...)} {
  if (interval_ < 1) {
    throw std::runtime_error("CronJob's interval is less than 1 second");
  }
}

template <typename _Callable, typename... _Args>
std::shared_ptr<Job> Scheduler::Run(int interval,
                                    _Callable&& function,
                                    _Args&& ... args) {
  return Add(true, interval,
             std::forward<_Callable>(function),
             std::forward<_Args>(args)...);
}

template <typename _Callable, typename... _Args>
std::shared_ptr<Job> Scheduler::RunOnce(int interval,
                                        _Callable&& function,
                                        _Args&& ... args) {
  return Add(false, interval,
             std::forward<_Callable>(function),
             std::forward<_Args>(args)...);
}

template <typename _Callable, typename... _Args>
std::shared_ptr<Job> Scheduler::Add(bool reiterable, int interval,
                                    _Callable&& function,
                                    _Args&& ... args) {
  std::lock_guard<std::mutex> lg(lock_);
  
  auto it = jobs_.emplace(
      current_time_ + interval,
      std::make_shared<Job>(reiterable, interval,
                            std::forward<_Callable>(function),
                            std::forward<_Args>(args)...));
  
  return it->second;
}
} // namespace cronjob