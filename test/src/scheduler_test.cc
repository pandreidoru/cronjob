//
// Created by Andrei - Doru Pata on 16/06/18.
//
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <scheduler_test.h>

#include <thread>

BOOST_FIXTURE_TEST_SUITE(SchedulerSuite, SchedulerFixture)

BOOST_AUTO_TEST_SUITE(Single_thread)

BOOST_AUTO_TEST_SUITE(Add)

BOOST_AUTO_TEST_CASE(One_job) {
  auto job = scheduler.Add(true, 1, [] {});
  
  BOOST_TEST(job != nullptr);
}

BOOST_AUTO_TEST_CASE(Jobs_with_same_signature) {
  auto job1 = scheduler.Add(true, 1, [] {});
  auto job2 = scheduler.Add(true, 1, [] {});
  
  BOOST_TEST(job1 != job2);
}

BOOST_AUTO_TEST_SUITE_END() // Add

BOOST_AUTO_TEST_SUITE(Remove)

BOOST_AUTO_TEST_CASE(Sets_nullptr) {
  auto job1 = scheduler.Add(true, 1, [] {});
  auto job2 = scheduler.Add(true, 2, [] {});
  
  scheduler.Remove(job1);
  scheduler.Remove(job2);
  
  BOOST_TEST(job1 == nullptr);
  BOOST_TEST(job2 == nullptr);
}

BOOST_AUTO_TEST_CASE(Double_remove) {
  auto job = scheduler.Add(true, 1, [] {});
  
  scheduler.Remove(job);
  BOOST_TEST(job == nullptr);
  
  scheduler.Remove(job);
  BOOST_TEST(job == nullptr);
}

BOOST_AUTO_TEST_SUITE_END() // Remove

BOOST_AUTO_TEST_SUITE(OnNewTime)

BOOST_AUTO_TEST_SUITE(Same_time)

BOOST_AUTO_TEST_CASE(Empty_scheduler) {
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 0, 0 }));
  
  auto count_fired = int{};
  
  auto job = scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 1);
  
  scheduler.Remove(job);
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 1);
}

BOOST_AUTO_TEST_CASE(Initial_time) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 0, 0 });
  scheduler.OnNewTime({ 0, 0 });
  scheduler.OnNewTime({ 0, 0 });
  
  BOOST_TEST(count_fired == 0);
}

BOOST_AUTO_TEST_CASE(Not_initial_time) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 2, 0 });
  scheduler.OnNewTime({ 2, 0 });
  scheduler.OnNewTime({ 2, 0 });
  
  BOOST_TEST(count_fired == 1);
}

BOOST_AUTO_TEST_SUITE_END() // Same_time

BOOST_AUTO_TEST_SUITE(Past_time)

BOOST_AUTO_TEST_CASE(Empty_scheduler) {
  scheduler.OnNewTime({ 2, 0 });
  
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 1, 0 }));
  
  auto count_fired = int{};
  
  auto job = scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 1);
  
  scheduler.Remove(job);
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 1);
}

BOOST_AUTO_TEST_CASE(Multiple_jobs) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 2);
  
  scheduler.OnNewTime({ 5, 0 });
  BOOST_TEST(count_fired == 2);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 4);
}

BOOST_AUTO_TEST_SUITE_END() // Past_time

BOOST_AUTO_TEST_SUITE(Future_time)

BOOST_AUTO_TEST_CASE(Empty_scheduler) {
  BOOST_CHECK_NO_THROW(scheduler.OnNewTime({ 10, 0 }));
  
  auto count_fired = int{};
  
  auto job = scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 0);
  
  scheduler.Remove(job);
  scheduler.OnNewTime({ 4, 0 });
  
  BOOST_TEST(count_fired == 0);
}

BOOST_AUTO_TEST_CASE(No_job_fired) {
  auto count_fired = int{};
  
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.OnNewTime({ 1, 0 });
  
  BOOST_TEST(count_fired == 0);
}

BOOST_AUTO_TEST_CASE(All_jobs_fired) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.Add(true, 3, [&] { ++count_fired; });
  scheduler.Add(true, 4, [&] { ++count_fired; });
  scheduler.OnNewTime({ 10, 0 });
  
  BOOST_TEST(count_fired == 4);
}

BOOST_AUTO_TEST_CASE(Some_jobs_fired) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.Add(true, 10, [&] { ++count_fired; });
  scheduler.Add(true, 20, [&] { ++count_fired; });
  scheduler.OnNewTime({ 5, 0 });
  
  BOOST_TEST(count_fired == 2);
}

BOOST_AUTO_TEST_CASE(All_jobs_fired_multiple_times) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.Add(true, 3, [&] { ++count_fired; });
  scheduler.Add(true, 4, [&] { ++count_fired; });
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 4);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_fired == (4 + 4));
}

BOOST_AUTO_TEST_CASE(Some_jobs_fired_multiple_times) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.Add(true, 100, [&] { ++count_fired; });
  scheduler.Add(true, 200, [&] { ++count_fired; });
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 2);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_fired == (2 + 2));
}

BOOST_AUTO_TEST_SUITE_END() // Future_time

BOOST_AUTO_TEST_CASE(Mixed_times) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.Add(true, 2, [&] { ++count_fired; });
  scheduler.Add(true, 100, [&] { ++count_fired; });
  scheduler.Add(true, 200, [&] { ++count_fired; });
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 2);
  
  scheduler.OnNewTime({ 4, 0 });
  BOOST_TEST(count_fired == 4);
  
  scheduler.OnNewTime({ 6, 0 });
  BOOST_TEST(count_fired == 6);
  
  scheduler.OnNewTime({ 4, 0 });
  BOOST_TEST(count_fired == 6);
  
  scheduler.OnNewTime({ 2, 0 });
  BOOST_TEST(count_fired == 6);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 8);
  
  scheduler.OnNewTime({ 300, 0 });
  BOOST_TEST(count_fired == 12);
}

BOOST_AUTO_TEST_CASE(Job_that_adds_another_job) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] {
    ++count_fired;
    scheduler.Add(true, 2, [&] { ++count_fired; });
  });
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 1);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_fired == (2 + 1));
  
  scheduler.OnNewTime({ 30, 0 });
  BOOST_TEST(count_fired == (3 + 1 + 2));
  
  scheduler.OnNewTime({ 40, 0 });
  BOOST_TEST(count_fired == (4 + 1 + 2 + 3));
}

BOOST_AUTO_TEST_SUITE_END() // OnNewTime

BOOST_AUTO_TEST_SUITE_END() // Single_thread

BOOST_AUTO_TEST_SUITE(Multi_thread)

BOOST_AUTO_TEST_CASE(Different_threads_adds_jobs_main_thread_call_OnNewTime) {
  auto count_fired = int{};
  auto thread1 =
      std::thread([&] { scheduler.Add(true, 1, [&] { ++count_fired; }); });
  auto thread2 =
      std::thread([&] { scheduler.Add(true, 1, [&] { ++count_fired; }); });
  auto thread3 =
      std::thread([&] { scheduler.Add(true, 2, [&] { ++count_fired; }); });
  auto thread4 =
      std::thread([&] { scheduler.Add(true, 3, [&] { ++count_fired; }); });
  
  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 4);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_fired == 8);
  
  scheduler.OnNewTime({ 20, 0 });
  BOOST_TEST(count_fired == 8);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 8);
  
  scheduler.OnNewTime({ 0, 0 });
  BOOST_TEST(count_fired == 8);
  
  scheduler.OnNewTime({ 10, 0 });
  BOOST_TEST(count_fired == 12);
}

BOOST_AUTO_TEST_CASE(OnNewTime_interleave) {
  auto thread1 = std::thread([&] { scheduler.Add(true, 1, [] {}); });
  auto thread2 = std::thread([&] { scheduler.Add(true, 2, [] {}); });
  auto thread3 = std::thread([&] { scheduler.Add(true, 3, [] {}); });
  auto thread4 = std::thread([&] { scheduler.Add(true, 4, [] {}); });
  auto thread5 = std::thread([&] { scheduler.Add(true, 5, [] {}); });
  
  auto thread6 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 60, 0 });
  });
  
  auto thread7 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 70, 0 });
  });
  
  auto thread8 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 80, 0 });
  });
  
  auto thread9 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 90, 0 });
  });
  
  auto thread10 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 100, 0 });
  });
  
  auto thread11 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    scheduler.OnNewTime({ 110, 0 });
  });
  
  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  thread5.join();
  thread6.join();
  thread7.join();
  thread8.join();
  thread9.join();
  thread10.join();
  thread11.join();
  
  BOOST_TEST(true);
}

BOOST_AUTO_TEST_SUITE_END() // Multi_thread

BOOST_AUTO_TEST_CASE(Resolution_of_one_second) {
  auto count_fired = int{};
  
  scheduler.Add(true, 1, [&] { ++count_fired; });
  scheduler.OnNewTime({ 2, 0 });
  scheduler.OnNewTime({ 2, 1 });
  scheduler.OnNewTime({ 2, 10 });
  
  BOOST_TEST(count_fired == 1);
}

BOOST_AUTO_TEST_SUITE_END() // SchedulerSuite