//
// Created by Andrei - Doru Pata on 18/06/18.
//
#include <scheduler.h>
#include <algorithm>

namespace cronjob {

void Scheduler::OnNewTime(const timeval& time) {
  std::list<std::shared_ptr<Job>> jobs_to_run;
  {
    std::lock_guard<std::mutex> lg(lock);
    
    if (current_time_ == time.tv_sec) {
      return;
    }
    
    auto old_time = current_time_;
    current_time_ = time.tv_sec;
    
    if (jobs.empty()) {
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
  decltype(jobs) new_jobs;
  
  for (const auto& time_job : jobs) {
    if (time_job.second->reiterable_) {
      new_jobs.emplace(current_time_ + time_job.second->interval_,
                       std::move(time_job.second));
    }
  }
  jobs.clear();
  jobs = std::move(new_jobs);
}

// Get lock before calling this method.
void Scheduler::FutureReschedule(
    const std::list<std::shared_ptr<Job>>& jobs_to_run) {
  auto end_of_jobs_to_run = jobs.upper_bound(current_time_);
  jobs.erase(jobs.begin(), end_of_jobs_to_run);
  
  for (const auto& job : jobs_to_run) {
    if (job->reiterable_) {
      jobs.emplace(current_time_ + job->interval_, job);
    }
  }
}

// Get lock before calling this method.
void Scheduler::GetJobsToRun(
    std::list<std::shared_ptr<Job>>& jobs_to_run) const {
  
  // Future time => Run once all jobs between old_time and current_time_.
  auto end_of_jobs_to_run = jobs.upper_bound(current_time_);
  
  // Backup jobs to be run.
  for (auto time_job = jobs.begin();
       time_job != end_of_jobs_to_run;
       ++time_job) {
    jobs_to_run.push_back(time_job->second);
  }
}

void Scheduler::Remove(std::shared_ptr<Job>& job) {
  if (job) {
    std::lock_guard<std::mutex> lg(lock);
    auto range = jobs.equal_range(current_time_ + job->interval_);
    for (auto iter = range.first; iter != range.second; ++iter) {
      if (iter->second == job) {
        jobs.erase(iter);
        break;
      }
    }
    job.reset();
  }
}
} // namespace cronjob
