//
// Created by Andrei - Doru Pata on 16/06/18.
//
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "scheduler.h"

#include <thread>

struct SchedulerFixture {
  cronjob::Scheduler scheduler;
};

BOOST_FIXTURE_TEST_SUITE(SchedulerSuite, SchedulerFixture)

BOOST_AUTO_TEST_SUITE(Single_thread)

BOOST_AUTO_TEST_SUITE(Add)

BOOST_AUTO_TEST_CASE(Jobs_with_same_signature_are_unique) {
  auto job1 = scheduler.Run(1, [] {});
  auto job2 = scheduler.Run(1, [] {});
  auto job3 = scheduler.RunOnce(1, [] {});
  
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
  BOOST_TEST(job3 != nullptr);
  BOOST_TEST(job1 != job2);
  BOOST_TEST(job1 != job3);
  BOOST_TEST(job2 != job3);
}

BOOST_AUTO_TEST_SUITE_END() // Run

BOOST_AUTO_TEST_SUITE(Remove)

BOOST_AUTO_TEST_CASE(Sets_jobs_with_nullptr) {
  auto job1 = scheduler.Run(1, [] {});
  auto job2 = scheduler.Run(2, [] {});
  auto job3 = scheduler.RunOnce(3, [] {});
  
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  scheduler.Remove(job3);
  
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
  BOOST_TEST(job3 == nullptr);
}

BOOST_AUTO_TEST_CASE(The_correct_job) {
  auto job1 = scheduler.Run(1, [] {});
  auto job2 = scheduler.Run(2, [] {});
  auto job3 = scheduler.RunOnce(3, [] {});
  auto job4 = scheduler.RunOnce(4, [] {});
  
  scheduler.Remove(job1);
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 != nullptr);
  BOOST_TEST(job3 != nullptr);
  BOOST_TEST(job4 != nullptr);
  
  scheduler.Remove(job3);
  
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 != nullptr);
  BOOST_TEST(job3 == nullptr);
  BOOST_TEST(job4 != nullptr);
}

BOOST_AUTO_TEST_CASE(Double_remove_does_not_throw_exception) {
  auto job1 = scheduler.Run(1, [] {});
  auto job2 = scheduler.RunOnce(1, [] {});
  
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
  
  BOOST_CHECK_NO_THROW(scheduler.Remove(job1));
  BOOST_TEST(job1 == nullptr);
  
  BOOST_CHECK_NO_THROW(scheduler.Remove(job2));
  BOOST_TEST(job2 == nullptr);
}

BOOST_AUTO_TEST_CASE(
    Remove_on_invalid_reference_to_a_RunOnce_job_set_it_to_nullptr) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.OnNewTime({ 10, 0 });
  
  auto job1 = scheduler.RunOnce(1, [&] { ++count_executed1; });
  auto job2 = scheduler.RunOnce(2, [&] { ++count_executed2; });
  
  scheduler.OnNewTime({ 5, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
  
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
}

BOOST_AUTO_TEST_SUITE_END() // Remove

BOOST_AUTO_TEST_SUITE(OnNewTime)

BOOST_AUTO_TEST_SUITE(Same_time)

BOOST_AUTO_TEST_CASE(On_empty_scheduler_does_nothing) {
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 0, 0 }));
  
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  auto job1 = scheduler.Run(1, [&] { ++count_executed1; });
  auto job2 = scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 5, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
}

BOOST_AUTO_TEST_CASE(After_initial_time_does_nothing) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 0, 0 });
  scheduler.OnNewTime({ 0, 0 });
  
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
}

BOOST_AUTO_TEST_CASE(After_not_initial_time_does_nothing) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
}

BOOST_AUTO_TEST_SUITE_END() // Same_time

BOOST_AUTO_TEST_SUITE(Past_time)

BOOST_AUTO_TEST_CASE(On_empty_scheduler_does_nothing) {
  scheduler.OnNewTime({ 10, 0 });
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 5, 0 }));
}

BOOST_AUTO_TEST_CASE(Does_not_fire_any_job_but_reschedules_repetitive_jobs) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  auto count_executed3 = 0;
  auto count_executed4 = 0;
  
  scheduler.OnNewTime({ 10, 0 });
  
  auto job1 = scheduler.Run(1, [&] { ++count_executed1; });
  auto job2 = scheduler.Run(2, [&] { ++count_executed2; });
  auto job3 = scheduler.RunOnce(3, [&] { ++count_executed3; });
  auto job4 = scheduler.RunOnce(4, [&] { ++count_executed4; });
  
  scheduler.OnNewTime({ 5, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
}

BOOST_AUTO_TEST_CASE(
    Removes_RunOnce_jobs_just_from_scheduler_reference_remains_invalid) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.OnNewTime({ 10, 0 });
  
  auto job1 = scheduler.RunOnce(1, [&] { ++count_executed1; });
  auto job2 = scheduler.RunOnce(2, [&] { ++count_executed2; });
  
  scheduler.OnNewTime({ 5, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
}

BOOST_AUTO_TEST_SUITE_END() // Past_time

BOOST_AUTO_TEST_SUITE(Future_time)

BOOST_AUTO_TEST_CASE(On_empty_scheduler_does_nothing) {
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 10, 0 }));
  
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  auto job1 = scheduler.Run(1, [&] { ++count_executed1; });
  auto job2 = scheduler.RunOnce(1, [&] { ++count_executed2; });
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
}

BOOST_AUTO_TEST_CASE(No_job_executed_if_new_time_is_too_early) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.Run(10, [&] { ++count_executed1; });
  scheduler.RunOnce(20, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 5, 0 });
  
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
}

BOOST_AUTO_TEST_CASE(No_reschedule_if_new_time_is_too_early) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.Run(10, [&] { ++count_executed1; });
  scheduler.RunOnce(20, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 5, 0 });
  
  BOOST_TEST(count_executed1 == 0);
  BOOST_TEST(count_executed2 == 0);
  
  scheduler.OnNewTime({ 100, 0 });
  
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
}

BOOST_AUTO_TEST_CASE(Jobs_are_executed_just_once) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.OnNewTime({ 10, 0 });
  
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
}

BOOST_AUTO_TEST_CASE(Only_expired_jobs_are_executed) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  auto count_executed3 = 0;
  auto count_executed4 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.Run(2, [&] { ++count_executed2; });
  scheduler.Run(10, [&] { ++count_executed3; });
  scheduler.Run(20, [&] { ++count_executed4; });
  scheduler.OnNewTime({ 5, 0 });
  
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
}

BOOST_AUTO_TEST_CASE(Only_repetitive_jobs_are_rescheduled_after_executed_once) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  auto count_executed3 = 0;
  auto count_executed4 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.Run(30, [&] { ++count_executed3; });
  scheduler.RunOnce(40, [&] { ++count_executed4; });
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_executed1 == 2);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
}

BOOST_AUTO_TEST_SUITE_END() // Future_time

BOOST_AUTO_TEST_CASE(Mixed_times) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  auto count_executed3 = 0;
  auto count_executed4 = 0;
  
  scheduler.Run(1, [&] { ++count_executed1; });
  scheduler.RunOnce(2, [&] { ++count_executed2; });
  scheduler.Run(100, [&] { ++count_executed3; });
  scheduler.Run(200, [&] { ++count_executed4; });
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 4, 0 });
  BOOST_TEST(count_executed1 == 2);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 3, 0 });
  BOOST_TEST(count_executed1 == 2);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed1 == 2);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 3);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 0);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 300, 0 });
  BOOST_TEST(count_executed1 == 4);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 1);
  BOOST_TEST(count_executed4 == 1);
}

BOOST_AUTO_TEST_CASE(Jobs_could_add_other_jobs) {
  auto count_repetitive1 = 0;
  auto count_repetitive2 = 0;
  auto count_once = 0;
  
  scheduler.Run(1, [&] {
    ++count_repetitive1;
    scheduler.Run(10, [&] { ++count_repetitive1; });
  });
  
  scheduler.Run(2, [&] {
    ++count_repetitive2;
    scheduler.Run(20, [&] { ++count_repetitive2; });
  });
  
  scheduler.RunOnce(1, [&] {
    ++count_once;
    scheduler.RunOnce(10, [&] { ++count_once; });
  });
  
  scheduler.OnNewTime({ 100, 0 });
  BOOST_TEST(count_repetitive1 == 1);
  BOOST_TEST(count_repetitive2 == 1);
  BOOST_TEST(count_once == 1);
  
  scheduler.OnNewTime({ 200, 0 });
  BOOST_TEST(count_repetitive1 == (2 + 1));
  BOOST_TEST(count_repetitive2 == (2 + 1));
  BOOST_TEST(count_once == (1 + 1));
  
  scheduler.OnNewTime({ 300, 0 });
  BOOST_TEST(count_repetitive1 == (3 + 1 + 2));
  BOOST_TEST(count_repetitive2 == (3 + 1 + 2));
  BOOST_TEST(count_once == (1 + 1));
  
  scheduler.OnNewTime({ 400, 0 });
  BOOST_TEST(count_repetitive1 == (4 + 1 + 2 + 3));
  BOOST_TEST(count_repetitive2 == (4 + 1 + 2 + 3));
  BOOST_TEST(count_once == (1 + 1));
}

BOOST_AUTO_TEST_CASE(Job_could_remove_another_job) {
  std::list<std::shared_ptr<cronjob::Job>> jobs;
  
  jobs.push_back(scheduler.Run(1, [&] {}));
  jobs.push_back(scheduler.Run(2, [&] {}));
  BOOST_TEST(jobs.size() == 2);
  
  scheduler.Run(20, [&] {
    scheduler.Remove(jobs.front());
    jobs.pop_front();
  });
  BOOST_TEST(jobs.size() == 2);
  
  scheduler.OnNewTime({ 100, 0 });
  BOOST_TEST(jobs.size() == 1);
  
  scheduler.OnNewTime({ 200, 0 });
  BOOST_TEST(jobs.size() == 0);
}

BOOST_AUTO_TEST_CASE(Resolution_is_of_one_second) {
  auto count_executed = 0;
  
  scheduler.Run(1, [&] { ++count_executed; });
  scheduler.OnNewTime({ 0, 10 });
  BOOST_TEST(count_executed == 0);
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_executed == 1);
  
  scheduler.OnNewTime({ 2, 1 });
  BOOST_TEST(count_executed == 1);
  
  scheduler.OnNewTime({ 2, 10 });
  BOOST_TEST(count_executed == 1);
}

BOOST_AUTO_TEST_SUITE_END() // OnNewTime

BOOST_AUTO_TEST_SUITE_END() // Single_thread

BOOST_AUTO_TEST_SUITE(Multiple_threads)

BOOST_AUTO_TEST_SUITE(Add)

BOOST_AUTO_TEST_CASE(Add_jobs_is_safe) {
  auto count_executed1 = 0;
  auto count_executed2 = 0;
  auto count_executed3 = 0;
  auto count_executed4 = 0;
  
  auto thread1 =
      std::thread([&] { scheduler.Run(1, [&] { ++count_executed1; }); });
  auto thread2 =
      std::thread([&] { scheduler.RunOnce(1, [&] { ++count_executed2; }); });
  auto thread3 =
      std::thread([&] { scheduler.Run(2, [&] { ++count_executed3; }); });
  auto thread4 =
      std::thread([&] { scheduler.Run(20, [&] { ++count_executed4; }); });
  
  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_executed1 == 1);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 1);
  BOOST_TEST(count_executed4 == 0);
  
  scheduler.OnNewTime({ 30, 0 });
  BOOST_TEST(count_executed1 == 2);
  BOOST_TEST(count_executed2 == 1);
  BOOST_TEST(count_executed3 == 2);
  BOOST_TEST(count_executed4 == 1);
}

BOOST_AUTO_TEST_SUITE_END() // Run

BOOST_AUTO_TEST_SUITE(Remove)

BOOST_AUTO_TEST_CASE(Sets_jobs_with_nullptr) {
  auto job1 = scheduler.Run(1, [&] {});
  auto job2 = scheduler.RunOnce(1, [&] {});
  
  auto thread1 = std::thread([&] { scheduler.Remove(job1); });
  auto thread2 = std::thread([&] { scheduler.Remove(job2); });
  
  thread1.join();
  thread2.join();
  
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
}

BOOST_AUTO_TEST_CASE(The_correct_job) {
  auto job1 = scheduler.Run(1, [&] {});
  auto job2 = scheduler.RunOnce(1, [&] {});
  auto job3 = scheduler.Run(1, [&] {});
  auto job4 = scheduler.RunOnce(1, [&] {});
  
  auto thread1 = std::thread([&] { scheduler.Remove(job2); });
  auto thread2 = std::thread([&] { scheduler.Remove(job4); });
  
  thread1.join();
  thread2.join();
  
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 == nullptr);
  BOOST_TEST(job3 != nullptr);
  BOOST_TEST(job4 == nullptr);
}

BOOST_AUTO_TEST_CASE(Double_remove_does_not_throw_exception) {
  auto job1 = scheduler.Run(1, [&] {});
  auto job2 = scheduler.RunOnce(1, [&] {});
  
  auto thread1 = std::thread([&] { scheduler.Remove(job1); });
  auto thread2 = std::thread([&] { scheduler.Remove(job1); });
  auto thread3 = std::thread([&] { scheduler.Remove(job2); });
  auto thread4 = std::thread([&] { scheduler.Remove(job2); });
  
  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
}

BOOST_AUTO_TEST_SUITE_END() // Remove

BOOST_AUTO_TEST_SUITE(OnNewTime)

BOOST_AUTO_TEST_CASE(Concurent_calling_is_safe) {
  auto count_repetitive = 0;
  auto count_once = 0;
  auto add_thread =
      std::thread([&] {
        scheduler.Run(1, [&] {
          ++count_repetitive;
          // Waits to allow onnewtime_thread2 to overlap onnewtime_thread1.
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
        scheduler.RunOnce(1, [&] {
          ++count_once;
          // Waits to allow onnewtime_thread2 to overlap onnewtime_thread1.
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
      });
  
  auto onnewthread_thread1 = std::thread([&] {
    // Waits for add_thread to add the jobs.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    scheduler.OnNewTime({ 10, 0 });
  });
  
  auto onnewthread_thread2 = std::thread([&] {
    // Waits for add_thread to add the job and onnewthread_thread1.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    scheduler.OnNewTime({ 20, 0 });
  });
  
  add_thread.join();
  onnewthread_thread1.join();
  onnewthread_thread2.join();
  
  BOOST_TEST(count_repetitive == 2);
  BOOST_TEST(count_once == 1);
  
  scheduler.OnNewTime({ 30, 0 });
  BOOST_TEST(count_repetitive == 3);
  BOOST_TEST(count_once == 1);
}

BOOST_AUTO_TEST_CASE(While_another_thread_calls_Add_is_safe) {
  auto count_repetitive1 = 0;
  auto count_repetitive2 = 0;
  auto count_once1 = 0;
  auto count_once2 = 0;
  
  std::shared_ptr<cronjob::Job> job1;
  std::shared_ptr<cronjob::Job> job2;
  std::shared_ptr<cronjob::Job> job3;
  std::shared_ptr<cronjob::Job> job4;
  
  auto add_thread1 = std::thread([&] {
    job1 = scheduler.Run(1, [&] {
      ++count_repetitive1;
      // Waits to allow add_thread to overlap onnewthread_thread.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    job3 = scheduler.RunOnce(1, [&] {
      ++count_once1;
      // Waits to allow add_thread to overlap onnewthread_thread.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
  });
  
  auto onnewthread_thread = std::thread([&] {
    // Waits for add_thread1 to add the jobs.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    scheduler.OnNewTime({ 10, 0 });
  });
  
  auto add_thread2 = std::thread([&] {
    // Waits for add_thread1 to add the job and onnewthread_thread.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    job2 = scheduler.Run(2, [&] { ++count_repetitive2; });
    job4 = scheduler.RunOnce(2, [&] { ++count_once2; });
  });
  
  add_thread1.join();
  onnewthread_thread.join();
  add_thread2.join();
  
  BOOST_TEST(count_repetitive1 == 1);
  BOOST_TEST(count_repetitive2 == 0);
  BOOST_TEST(count_once1 == 1);
  BOOST_TEST(count_once2 == 0);
  BOOST_TEST(job1 != nullptr);
  BOOST_TEST(job2 != nullptr);
  BOOST_TEST(job3 != nullptr);
  BOOST_TEST(job4 != nullptr);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_repetitive1 == 2);
  BOOST_TEST(count_repetitive2 == 1);
  BOOST_TEST(count_once1 == 1);
  BOOST_TEST(count_once2 == 1);
}

BOOST_AUTO_TEST_CASE(While_another_thread_calls_Remove_is_safe) {
  auto count_repetitive = 0;
  auto count_once = 0;
  
  std::shared_ptr<cronjob::Job> job_repetitive;
  std::shared_ptr<cronjob::Job> job_once;
  
  auto add_thread = std::thread([&] {
    job_repetitive = scheduler.Run(1, [&] {
      ++count_repetitive;
      // Waits to allow remove_thread to overlap onnewthread_thread.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    job_once = scheduler.Run(1, [&] {
      ++count_once;
      // Waits to allow remove_thread to overlap onnewthread_thread.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
  });
  
  auto onnewthread_thread = std::thread([&] {
    // Waits for add_thread to add the jobs.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    scheduler.OnNewTime({ 10, 0 });
  });
  
  auto remove_thread = std::thread([&] {
    // Waits for add_thread to add the job and onnewthread_thread.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    scheduler.Remove(job_repetitive);
    scheduler.Remove(job_once);
  });
  
  add_thread.join();
  onnewthread_thread.join();
  remove_thread.join();
  
  BOOST_TEST(count_repetitive == 1);
  BOOST_TEST(count_once == 1);
  BOOST_TEST(job_repetitive == nullptr);
  BOOST_TEST(job_once == nullptr);
}

BOOST_AUTO_TEST_SUITE_END() // OnNewTime

BOOST_AUTO_TEST_SUITE(Stress)

/* Two threads are adding jobs to Scheduler, storing references in two separate
 * lists. Other two threads takes jobs from the front of those lists and remove
 * them from the Scheduler.
 * Other three threads are calling OnNewTime to move the Scheduler current_time_
 * forward and backward.
 */
BOOST_AUTO_TEST_CASE(Stress1) {
  try {
    auto iterations = 1000;
    auto resolution = 1;
    std::mutex lock1;
    std::mutex lock2;
    std::list<std::shared_ptr<cronjob::Job>> jobs1;
    std::list<std::shared_ptr<cronjob::Job>> jobs2;
    
    auto adding_thread1 = std::thread([&] {
      for (auto i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lg(lock1);
        jobs1.push_back(scheduler.Run((i % 10) + 1, [&] {}));
        jobs1.push_back(scheduler.RunOnce(((i * 2) % 10) + 1, [&] {}));
        std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      }
    });
    
    auto removing_thread1 = std::thread([&] {
      std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      for (auto i = 0; i < iterations; ++i) {
        {
          std::lock_guard<std::mutex> lg(lock1);
          if (!jobs1.empty()) {
            scheduler.Remove(jobs1.front());
            jobs1.pop_front();
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2 * resolution));
      }
    });
    
    auto adding_thread2 = std::thread([&] {
      for (auto i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lg(lock2);
        jobs2.push_back(scheduler.Run((i % 5) + 1, [&] {}));
        jobs2.push_back(scheduler.RunOnce((i % 5) + 1, [&] {}));
        std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      }
    });
    
    auto removing_thread2 = std::thread([&] {
      std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      for (auto i = 0; i < iterations; ++i) {
        {
          std::lock_guard<std::mutex> lg(lock2);
          if (!jobs2.empty()) {
            scheduler.Remove(jobs2.front());
            jobs2.pop_front();
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2 * resolution));
      }
    });
    
    auto time_thread1 = std::thread([&] {
      for (auto i = 0; i < iterations; ++i) {
        scheduler.OnNewTime({ 10, 0 });
        std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      }
    });
    
    auto time_thread2 = std::thread([&] {
      for (auto i = 0; i < iterations; ++i) {
        scheduler.OnNewTime({ 30, 0 });
        std::this_thread::sleep_for(std::chrono::milliseconds(resolution));
      }
    });
    
    auto time_thread3 = std::thread([&] {
      for (auto i = 0; i < iterations; ++i) {
        scheduler.OnNewTime({ 50, 0 });
        std::this_thread::sleep_for(std::chrono::milliseconds(2 * resolution));
      }
    });
    
    adding_thread1.join();
    adding_thread2.join();
    removing_thread1.join();
    removing_thread2.join();
    time_thread1.join();
    time_thread2.join();
    time_thread3.join();
  }
  catch (...) {
    BOOST_TEST_FAIL("Exception thrown!");
  }
  BOOST_TEST(true);
}

BOOST_AUTO_TEST_SUITE_END() // Stress

BOOST_AUTO_TEST_SUITE_END() // Multi_thread

BOOST_AUTO_TEST_SUITE_END() // SchedulerSuite