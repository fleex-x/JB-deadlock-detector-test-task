#include "safe_mutex.h"
#include <mutex>
#include <thread>
#include <vector>

using test_task::safe_mutex;

void test_case1() {
	const int STEPS = 10'000;
    safe_mutex m1;
    safe_mutex m2;
    safe_mutex m3;
    safe_mutex m4;
    safe_mutex m5;

    std::thread t1([&]() {
        for (int i = 0; i < STEPS; ++i) {
            std::unique_lock l1(m1);
            std::unique_lock l2(m2);
        }
    });
    std::thread t2([&]() {
        for (int i = 0; i < STEPS; ++i) {
            std::unique_lock l1(m2);
            std::unique_lock l2(m3);
        }
    });
    std::thread t3([&]() {
        for (int i = 0; i < STEPS; ++i) {
            std::unique_lock l1(m3);
            std::unique_lock l2(m4);
        }
    });
    std::thread t4([&]() {
        for (int i = 0; i < STEPS; ++i) {
            std::unique_lock l1(m4);
            std::unique_lock l2(m5);    
        }
    });
    std::thread t5([&]() {
        for (int i = 0; i < STEPS; ++i) {
            std::unique_lock l1(m5);
            std::unique_lock l2(m1);    
        };
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}

int main() {
	test_case1();
	return 0;
}