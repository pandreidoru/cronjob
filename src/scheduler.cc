//
// Created by Andrei - Doru Pata on 18/06/18.
//
#include <scheduler.h>
#include <algorithm>

namespace cronjob {

void Scheduler::OnNewTime(const timeval& time) {
  std::list<std::shared_ptr<Job>> jobs_to_run;
  {
    std::lock_guard<std::mutex> lg(lock_);
    
    if (current_time_ == time.tv_sec) {
      return;
    }
    
    auto old_time = current_time_;
    current_time_ = time.tv_sec;
  
    if (jobs_.empty()) {
      return;
    }
    
    if (old_time > current_time_) {
      PastReschedule();
      return;
    } else {
      GetJobsToRun(jobs_to_run);
      FutureReschedule(jobs_to_run);
    }
  }
  
  for (const auto& job : jobs_to_run) {
    job->function_();
  }
}

// Get lock before calling this method.
void Scheduler::PastReschedule() {
  auto new_jobs = std::multimap<long, std::shared_ptr<Job>>{};
  
  for (auto& time_job : jobs_) {
    if (time_job.second->reiterable_) {
      new_jobs.emplace(current_time_ + time_job.second->interval_,
                       std::move(time_job.second));
    }
  }
  
  jobs_ = std::move(new_jobs);
}

// Get lock before calling this method.
void Scheduler::FutureReschedule(
    const std::list<std::shared_ptr<Job>>& jobs_to_run) {
  auto end_of_jobs_to_run = jobs_.upper_bound(current_time_);
  jobs_.erase(jobs_.begin(), end_of_jobs_to_run);
  
  for (const auto& job : jobs_to_run) {
    if (job->reiterable_) {
      jobs_.emplace(current_time_ + job->interval_, job);
    }
  }
}

// Get lock before calling this method.
void Scheduler::GetJobsToRun(
    std::list<std::shared_ptr<Job>>& jobs_to_run) const {
  auto end_of_jobs_to_run = jobs_.upper_bound(current_time_);
  
  // Backup jobs to be run.
  for (auto time_job = jobs_.begin();
       time_job != end_of_jobs_to_run;
       ++time_job) {
    jobs_to_run.push_back(time_job->second);
  }
}

void Scheduler::Remove(std::shared_ptr<Job>& job) {
  std::lock_guard<std::mutex> lg(lock_);
  if (job) {
    auto range = jobs_.equal_range(current_time_ + job->interval_);
    for (auto iter = range.first; iter != range.second; ++iter) {
      if (iter->second == job) {
        jobs_.erase(iter);
        break;
      }
    }
    job.reset();
  }
}
} // namespace cronjob
