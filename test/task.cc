// Copyright (c) 2020 - present, Hawtian Wang (twistoy.wang@gmail.com)
//

#include <catch2/catch_all.hpp>

#include <ce/concepts/awaiter_traits.hh>
#include <ce/concepts/is_coroutine_handle.hh>
#include <ce/impls/task.hh>

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

  REQUIRE_FALSE(concepts::awaiter<awaiter1>);

  struct awaiter2 {
    bool await_ready();
    void await_resume();
    void await_suspend(std::coroutine_handle<>);
  };

  REQUIRE(concepts::awaiter<awaiter2>);
}

TEST_CASE("do not exec before await", "[execute][task]") {
  bool started = false;

  auto func = [&]() -> Task<int> {
    std::cout << "func exec" << std::endl;
    started = true;
    co_return 10;
  };

  auto v = ht::sync_wait([&]() -> ht::task<int> {
    std::cout << "task exec" << std::endl;
    auto t = func();

    REQUIRE(!started);
    int x = (co_await t) * 2;
    REQUIRE(started);

    co_return x;
  }());

  REQUIRE(v == 20);
}

}  // namespace ce::testing
