# Cronjob Scheduler
Is an interface that fulfills following requirements:
- The CronJob's interface allows adding and removing of jobs at any time.
- The CronJob's internal "clock" is driven by a public *void onNewTime(const struct timeval&)* method. In other words, it shouldn't use the system time for the logic.
- In case the *timeval* passed to *onNewTime(timeval)* is 
    - the same as the one from the previous call, nothing will happen.
    - less than the one from the previous call (past time), no job will run, but all jobs will be rescheduled from that moment.
    - greater than the one from the previous call (future time), all jobs, that should been triggered in this period, will run just once.
- After a job is added, it is executed every X "seconds" based on whatever time is passed to *onNewTime(timeval)*. The interval X is configurable per job.
- The interface must be thread-safe.
- There is only one Scheduler.
- All jobs with expired time should run in the thread that called *onNewTime(timeval)*.
- Any job could add another job.
- A resolution of 1 second is acceptable.
- Job's period must be restricted to not be less than 1 (second).
- The interface doesn't support change of period of already added jobs.
- Nothing is happening in case of double job removing.

- This project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

# Used modules
- Unit testing
  - [Boost.Test](https://www.boost.org/doc/libs/1_66_0/libs/test/doc/html/index.html)
- Coverage
  - [cmake-modules](https://github.com/bilke/cmake-modules)
  - [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
  - [lcov](http://ltp.sourceforge.net/coverage/lcov.php)

# Cloning this repository
- You might need to install [git-subtree](https://github.com/apenwarr/git-subtree) before cloning this repository.

# Code Coverage
- The output of the code coverage executed with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) can be seen by opening the following file in a web browser:
> cmake-build-debug/test_coverage/index.html