//
// Created by Andrei - Doru Pata on 18/06/18.
//
#include <thread>
#include <scheduler.h>

static bool print_debug = true;
static int iterations = 10;
static unsigned int resolution = 1000;

void SetHighFrequency(bool enable) {
  if (enable) {
    print_debug = false;
    iterations = 1000;
    resolution = 1;
  } else {
    iterations = 10;
    resolution = 1000;
  }
}

void Sleep(int interval) {
  std::this_thread::sleep_for(std::chrono::milliseconds(interval * resolution));
}

void Print(std::ostream&) {}

template <typename T, typename... Args>
void Print(std::ostream& out, T t, Args... args) {
  if (print_debug) {
    out << t;
    Print(out, std::forward<Args>(args)...);
  }
}

void Stress() {
  cronjob::Scheduler scheduler;
  
  std::mutex lock1;
  std::mutex lock2;
  std::list<std::shared_ptr<cronjob::Job>> jobs1;
  std::list<std::shared_ptr<cronjob::Job>> jobs2;
  
  SetHighFrequency(false);
  
  auto adding_thread1 = std::thread([&] {
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout);
      Print(std::cout, "list1 + ", (i % 10) + 1, "\n");
      std::lock_guard<std::mutex> lg(lock1);
      jobs1.push_back(scheduler.Run((i % 10) + 1, [&] {}));
      jobs1.push_back(scheduler.RunOnce(((i * 2) % 10) + 1, [&] {}));
      Sleep(1);
    }
  });
  
  auto removing_thread1 = std::thread([&] {
    Sleep(1);
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "list1 - ", (i % 10) + 1, "\n");
      {
        std::lock_guard<std::mutex> lg(lock1);
        if (!jobs1.empty()) {
          scheduler.Remove(jobs1.front());
          jobs1.pop_front();
        }
      }
      Sleep(2);
    }
  });
  
  auto adding_thread2 = std::thread([&] {
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "list2 + ", (i % 5) + 1, "\n");
      std::lock_guard<std::mutex> lg(lock2);
      jobs2.push_back(scheduler.Run((i % 5) + 1, [&] {}));
      jobs2.push_back(scheduler.RunOnce((i % 5) + 1, [&] {}));
      Sleep(1);
    }
  });
  
  auto removing_thread2 = std::thread([&] {
    Sleep(1);
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "list2 - ", (i % 5) + 1, "\n");
      {
        std::lock_guard<std::mutex> lg(lock2);
        if (!jobs2.empty()) {
          scheduler.Remove(jobs2.front());
          jobs2.pop_front();
        }
      }
      Sleep(2);
    }
  });
  
  auto time_thread1 = std::thread([&] {
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "OnNewTime(10)", "\n");
      scheduler.OnNewTime({10, 0});
      Sleep(1);
    }
  });
  
  auto time_thread2 = std::thread([&] {
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "OnNewTime(30)", "\n");
      scheduler.OnNewTime({30, 0});
      Sleep(1);
    }
  });
  
  auto time_thread3 = std::thread([&] {
    for (auto i = 0; i < iterations; ++i) {
      Print(std::cout, "OnNewTime(50)", "\n");
      scheduler.OnNewTime({50, 0});
      Sleep(2);
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

int main() {
  Stress();
}
