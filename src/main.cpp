#include "safe_mutex.h"
#include <iostream>

using namespace test_task;

void test_case1() {
	safe_mutex m;
	std::unique_lock l(m);
	std::unique_lock l1(m);
}


int main() {
	test_case1();
	return 0;
}