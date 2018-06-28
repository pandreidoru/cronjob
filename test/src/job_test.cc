//
// Created by Andrei - Doru Pata on 16/06/18.
//
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "scheduler.h"

void Foo() {}

void FooParams(int, std::string) {}

class Boo {
 public:
  void operator()() {}
};

class BooParams {
 public:
  void operator()(int, std::string) {}
};

BOOST_AUTO_TEST_SUITE(JobSuite)

BOOST_AUTO_TEST_SUITE(Ctors)

BOOST_AUTO_TEST_SUITE(One_shoot)

BOOST_AUTO_TEST_CASE(Function) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, Foo));
}

BOOST_AUTO_TEST_CASE(Function_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, FooParams, 1, "test"));
}

BOOST_AUTO_TEST_CASE(Functor) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, Boo{}));
}

BOOST_AUTO_TEST_CASE(Functor_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, BooParams{}, 2, "test"));
}

BOOST_AUTO_TEST_CASE(Lambda) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, [] {}));
}

BOOST_AUTO_TEST_CASE(Lambda_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1,
                                    [](int, std::string) {}, 3, "test"));
}

BOOST_AUTO_TEST_CASE(Validate_interval) {
  BOOST_CHECK_NO_THROW(cronjob::Job(false, 1, [] {}));
  BOOST_CHECK_THROW(cronjob::Job(false, 0, [] {}), std::runtime_error);
  BOOST_CHECK_THROW(cronjob::Job(false, -1, [] {}), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END() // On_shoot

BOOST_AUTO_TEST_SUITE(Reiterable)

BOOST_AUTO_TEST_CASE(Function) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, Foo));
}

BOOST_AUTO_TEST_CASE(Function_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, FooParams, 1, "test"));
}

BOOST_AUTO_TEST_CASE(Functor) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, Boo{}));
}

BOOST_AUTO_TEST_CASE(Functor_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, BooParams{}, 2, "test"));
}

BOOST_AUTO_TEST_CASE(Lambda) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, [] {}));
}

BOOST_AUTO_TEST_CASE(Lambda_with_parameters) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1,
                                    [](int, std::string) {}, 3, "test"));
}

BOOST_AUTO_TEST_CASE(Validate_interval) {
  BOOST_CHECK_NO_THROW(cronjob::Job(true, 1, [] {}));
  BOOST_CHECK_THROW(cronjob::Job(true, 0, [] {}), std::runtime_error);
  BOOST_CHECK_THROW(cronjob::Job(true, -1, [] {}), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END() // Reiterable

BOOST_AUTO_TEST_SUITE_END() // Ctors

BOOST_AUTO_TEST_SUITE_END() // JobSuite
