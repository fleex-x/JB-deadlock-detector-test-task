#include "safe_mutex.h"
#include <thread>
#include <mutex>
#include <map>
#include <optional>
#include <cstddef>
#include <iostream>

namespace test_task {

bool mutex_graph::exists_cycle(std::thread::id vertex, 
	                           std::thread::id started_vertex, 
	                           bool is_first_step	) {
	if (!is_first_step && vertex == started_vertex) {
		return true;
	}
	if (current_expectation[vertex] == 0 || 
		!current_thread[current_expectation[vertex]].has_value()) {
		return false;
	} else {
		return exists_cycle(current_thread[current_expectation[vertex]].value(),
			                started_vertex,
			                false);
	}
}

void mutex_graph::add_new_expectation(std::thread::id thread_id, std::uint64_t mutex_id) {
	std::unique_lock l(m);
	current_expectation[thread_id] = mutex_id;
	if (exists_cycle(thread_id, thread_id)) {
		std::cerr << "Deadlock is discovered" << std::endl;
		exit(1);
	}
}

void mutex_graph::delete_expectation_and_add_new_locked_mutex(std::thread::id thread_id, std::uint64_t mutex_id) {
	std::unique_lock l(m);
	current_expectation[thread_id] = 0;
	current_thread[mutex_id] = thread_id;
}

void mutex_graph::delete_locked_mutex(std::uint64_t mutex_id) {
	std::unique_lock l(m);
	current_thread[mutex_id].reset();
}

safe_mutex::safe_mutex() : this_mutex_id(mutex_counter++) {
}

void safe_mutex::lock() {
	thread_dependencies.add_new_expectation(std::this_thread::get_id(), this_mutex_id);
	m.lock();
	thread_dependencies.delete_expectation_and_add_new_locked_mutex(std::this_thread::get_id(), this_mutex_id);
}

void safe_mutex::unlock() {
	thread_dependencies.delete_locked_mutex(this_mutex_id);
	m.unlock();
}

// safe_mutex::~safe_mutex() {

// }

} //test_task