#ifndef SAFE_MUTEX_H_
#define SAFE_MUTEX_H_
#include <thread>
#include <mutex>
#include <map>
#include <optional>
#include <cstddef>
#include <atomic>
#include <exception>

namespace test_task {

class deadlock_exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error; 
    //https://stackoverflow.com/questions/7667701/reusable-constructors-c
};

class mutex_graph {
private:
	std::mutex m;
	
	std::map<std::thread::id, std::uint64_t> current_expectation; 
	//current_expectaion[thread_id] -- the mutex this thread is waiting for
	
	std::map<std::uint64_t, std::optional<std::thread::id>> current_thread;
	//current_thread[mutex_id] -- the thread that is holding the mutex

	bool exists_cycle(std::thread::id vertex, 
	                  std::thread::id started_vertex, 
	                  bool is_first_step = true,
                      int depth = 0);
	//can only be called under the mutex  

public:
	void add_new_expectation(std::thread::id thread_id, std::uint64_t mutex_id);
	void delete_expectation_and_add_new_locked_mutex(std::thread::id thread_id, std::uint64_t mutex_id);
	void delete_locked_mutex(std::uint64_t mutex_id);
};

class safe_mutex {
private:
	inline static mutex_graph thread_dependencies;
	inline static std::atomic<std::uint64_t> mutex_counter = 1;
	const std::uint64_t this_mutex_id; 
	std::mutex m;
public:
	safe_mutex();

	void lock();
	void unlock();

	//~safe_mutex();
};

}

#endif //SAFE_MUTEX_H