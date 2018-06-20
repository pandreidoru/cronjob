# Cronjob Scheduler
The Cronjob Scheduler is a thread-safe Cronjob manager.

A Cronjob is a piece of code that is executed periodically at fixed intervals. It is often used for cleanup routines or simply for logic that needs to be executed at specific intervals.

# Interface instantiation
To use the Cronjob Scheduler the *scheduler.h* header must be included and the Scheduler class from the cronjob namespace must be instantiated
```C++
#include "scheduler.h"

cronjob::Scheduler scheduler;
```
### Types of jobs
- Two types of jobs are supported
    - **One-shot job** - Executed once at the end of the configured interval. These jobs are added to the Scheduler by calling *RunOnce(interval, callable_function, list_of_arguments)*.
    - **Repetitive job** - Executed every time the configured interval expires. These jobs are added to the Scheduler by calling *Run(interval, callable_function, list_of_arguments)*.

### Adding and removing jobs
- The CronJob's interface allows adding and removing of jobs at any time (from any thread and even from another job).
- Each type of job could be added to the Scheduler by its dedicated method
```C++
// Add One-shot jobs.
std::shared_ptr<Job> Scheduler::RunOnce(int interval, _Callable&& function, _Args&& ... args);

// Add repetitive jobs.
std::shared_ptr<Job> Scheduler::Run(int interval, _Callable&& function, _Args&& ... args);
```
- These methods returns pointers to the added jobs to allow clients to remove them later by calling
```C++
void Remove(std::shared_ptr<Job>& job);
```
- The pointers to the One-shot jobs are invalidated if the Scheduler executes the jobs or the *OnNewTime(timeval)* method is called with a past time. In this case the pointer could be set to *nullptr* manually or by calling *Remove(job)* method. 
- Nothing is happening in case of double job removing.

### Internal clock
- The CronJob's internal "clock" is driven by a public *void onNewTime(const struct timeval&)* method. In other words, it doesn't use the system time for the logic.
- *onNewTime(timeval)* could be called with three types of values
    - the same value as the one from the previous call. In this case nothing is happening.
    - a value smaller than the one from the previous call (past time). In this case no job will run, but all repetitive jobs will be rescheduled based on the new value. All one-shot jobs are removed from the scheduler and the pointers become invalid.
    - a value greater than one from the previous call (future time). In this case all jobs, that should been triggered in this period, will run just once and the repetitive ones are rescheduled.
- All jobs with expired time run in the thread that called *onNewTime(timeval)*.
- The time resolution is of 1 second.
- Job's period are restricted to not be less than 1 (second).
- The interface doesn't support change of periods of already added jobs.

### Project
- The Following modules were used
    - Unit testing
        - [Boost.Test](https://www.boost.org/doc/libs/1_66_0/libs/test/doc/html/index.html)
    - Code coverage
        - [cmake-modules](https://github.com/bilke/cmake-modules)
        - [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
        - [lcov](http://ltp.sourceforge.net/coverage/lcov.php)

- The output of the code coverage executed with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) can be seen by opening the following file in a web browser:
> cmake-build-debug/test_coverage/index.html

- This project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
