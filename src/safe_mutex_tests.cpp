//https://github.com/onqtam/doctest/tree/master/doc/markdown
#include "safe_mutex.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include "doctest.h"
#include <iostream>


using test_task::safe_mutex;
using test_task::deadlock_exception;

namespace {
class latch {
private:
    std::mutex m;
    int counter;
    std::condition_variable counter_changed;

public:
    explicit latch(int counter_) : counter(counter_) {
    }
    void arrive_and_wait() {
        std::unique_lock l(m);
        counter--;
        counter_changed.notify_all();
        counter_changed.wait(l, [&]() { return counter <= 0; });
    }
};
}  // namespace
//Useful for tests to run threads approximately at the same time

TEST_CASE("Small test without deadlocks") {
    safe_mutex m;
    std::unique_lock l(m);
    l.unlock();
    l.lock();
    l.unlock();
    l.lock();
    l.unlock();
}

TEST_CASE("Re-lock in the same thread") {
    safe_mutex m;
    std::unique_lock l1(m);
    CHECK_THROWS_AS(m.lock(), deadlock_exception);
}

TEST_CASE("High probability of deadlock-1") {
    const int STEPS = 10'000;
    latch latch(4);
    safe_mutex m1;
    safe_mutex m2;
    safe_mutex m3;
    safe_mutex m4;
    std::atomic<std::size_t> counter = 0;

    std::thread t1([&]() {
        latch.arrive_and_wait();
        for (int i = 0; i < STEPS; ++i) {
            try {
                std::unique_lock l1(m1);
                std::unique_lock l2(m2);
            } catch(deadlock_exception &) {
                ++counter;
            }
        }
    });
    std::thread t2([&]() {
        latch.arrive_and_wait();
        for (int i = 0; i < STEPS; ++i) {
            try {
                std::unique_lock l1(m2);
                std::unique_lock l2(m3);
            } catch(deadlock_exception &) {
                ++counter;
            }
        }
    });
    std::thread t3([&]() {
        latch.arrive_and_wait();
        for (int i = 0; i < STEPS; ++i) {
            try {
                std::unique_lock l1(m3);
                std::unique_lock l2(m4);
            } catch(deadlock_exception &) {
                ++counter;
            }
        }
    });
    std::thread t4([&]() {
        latch.arrive_and_wait();
        for (int i = 0; i < STEPS; ++i) {
            try {
                std::unique_lock l1(m4);
                std::unique_lock l2(m1);
            } catch(deadlock_exception &) {
                ++counter;
            }
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    WARN(counter > 0);
}

TEST_CASE("High probability of deadlock-2") {
    const int STEPS = 1000;
    const int BIG_STEPS = 100;
    for (int i = 0; i < BIG_STEPS; ++i) {
        latch latch(4);
        safe_mutex m1;
        safe_mutex m2;
        safe_mutex m3;
        safe_mutex m4;
        std::atomic<std::size_t> counter = 0;

        std::thread t1([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m1);
                    std::unique_lock l2(m2);
                    std::unique_lock l3(m3);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t2([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m2);
                    std::unique_lock l2(m3);
                    std::unique_lock l3(m4);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t3([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m3);
                    std::unique_lock l2(m4);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t4([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m4);
                    std::unique_lock l2(m1);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        WARN(counter > 0);
    }
}

TEST_CASE("No deadlocks, common data") {
    const int STEPS = 1000;
    const int BIG_STEPS = 100;
    for (int i = 0; i < BIG_STEPS; ++i) {
        latch latch(4);
        safe_mutex m1;
        safe_mutex m2;
        safe_mutex m3;
        safe_mutex m4;
        std::vector <int> common_vec;
        std::atomic <std::size_t> counter = 0;

        std::thread t1([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m1);
                    std::unique_lock l2(m2);
                    std::unique_lock l3(m3);
                    std::unique_lock l4(m4);
                    common_vec.push_back(i);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t2([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l2(m2);
                    std::unique_lock l3(m3);
                    std::unique_lock l4(m4);
                    common_vec.push_back(i);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t3([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l3(m3);
                    std::unique_lock l4(m4);
                    common_vec.push_back(i);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });
        std::thread t4([&]() {
            latch.arrive_and_wait();
            for (int i = 0; i < STEPS; ++i) {
                try {
                    std::unique_lock l1(m4);
                    common_vec.push_back(i);
                } catch(deadlock_exception &) {
                    ++counter;
                }
            }
        });

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        CHECK(counter == 0);
    }
}