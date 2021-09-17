#include "safe_mutex.h"
#include <thread>
#include <mutex>
#include <map>
#include <optional>
#include <cstddef>
#include <iostream>
#include <exception>

namespace test_task {

bool mutex_graph::exists_cycle(std::thread::id vertex, 
                               std::thread::id started_vertex, 
                               bool is_first_step) {
    if (!is_first_step && vertex == started_vertex) {
        return true;
    }
    if (current_expectation.count(vertex) == 0 || 
        current_thread.count(current_expectation[vertex]) == 0) {
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
        current_expectation.erase(thread_id);
        #ifdef SAFE_MUTEX_TESTS_
           throw deadlock_exception("Deadlock is discovered");
        #else
            std::cerr << "Deadlock is discovered" << std::endl;
            std::exit(1);
        #endif
    }
}

void mutex_graph::delete_expectation_and_add_new_locked_mutex(std::thread::id thread_id, std::uint64_t mutex_id) {
    std::unique_lock l(m);
    current_expectation.erase(thread_id);
    current_thread[mutex_id] = thread_id;
}

void mutex_graph::delete_locked_mutex(std::uint64_t mutex_id) {
    std::unique_lock l(m);
    current_thread.erase(mutex_id);
}

multithread_counter::multithread_counter(std::uint64_t counter_) : counter(counter_) {
}

std::uint64_t multithread_counter::get_next_value() {
    std::unique_lock l(m);
    return counter++;
}

safe_mutex::safe_mutex() : this_mutex_id(mutex_counter.get_next_value()) {
}

void safe_mutex::lock() {
    thread_dependencies.add_new_expectation(std::this_thread::get_id(), this_mutex_id);
    m.lock();
    //check note1
    thread_dependencies.delete_expectation_and_add_new_locked_mutex(std::this_thread::get_id(), this_mutex_id);
}

void safe_mutex::unlock() {
    thread_dependencies.delete_locked_mutex(this_mutex_id);
    //check note2
    m.unlock();
}

// safe_mutex::~safe_mutex() {

// }

} //test_task