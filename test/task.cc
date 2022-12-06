// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <catch2/catch_all.hpp>

#include <ce/concepts/awaiter_traits.hh>
#include <ce/concepts/is_coroutine_handle.hh>
#include <ce/impls/helpers/sync_wait.hh>
#include <ce/impls/task.hh>
#include <ce/impls/tasks/sync_wait.hh>
#include <ce/impls/tasks/sync_wait_promise.hh>

namespace ce::testing {

TEST_CASE("coroutine handle type", "[traits]") {
  struct P;

  REQUIRE(concepts::is_coroutine_handle_v<std::coroutine_handle<>>);
  REQUIRE(concepts::is_coroutine_handle_v<std::coroutine_handle<P>>);
}

TEST_CASE("awaiter type", "[traits]") {
  struct awaiter1 {
    bool await_ready();
    void await_resume();
    void await_suspend();  // invalid
  };

  // REQUIRE_FALSE(concepts::awaiter<awaiter1>);

  struct awaiter2 {
    bool await_ready();
    void await_resume();
    void await_suspend(std::coroutine_handle<>);
  };

  REQUIRE(concepts::awaiter<awaiter2>);
}

TEST_CASE("do not exec before await", "[execute][task]") {
  bool started = false;

  auto func = [&]() -> task<int> {
    std::cout << "func exec" << std::endl;
    started = true;
    co_return 10;
  };

  std::cout << "before construct" << std::endl;
  auto t = func();
  REQUIRE_FALSE(started);

  std::cout << "before run" << std::endl;
  auto v = sync_wait(std::move(t));
  std::cout << "after run" << std::endl;

  REQUIRE(started);
  REQUIRE(v == 10);
}

TEST_CASE("lots of tasks", "[coroutine][task]") {
  auto return_one = []() -> ce::task<int> {
    co_return 1;
  };

  ce::sync_wait([](auto func) -> ce::task<> {
    int sum = 0;
    for (int i = 0; i < 1'000'000; ++i) {
      sum += co_await func();
    }
    REQUIRE(sum == 1'000'000);
  }(return_one));
}

}  // namespace ce::testing
