#ifndef SAFE_MUTEX_H_
#define SAFE_MUTEX_H_

namespace test_task {

class mutex_graph {
private:
	std::mutex m;
	
	std::map<std::thread::id, std::uint64_t> current_expectation; 
	//current_expectaion[thread_id] -- the mutex this thread is waiting for
	
	std::map<std::uint64_t, std::thread::id> current_thread;
	//current_thread[mutex_id] -- the thread that is holding the mutex

	bool exists_cycle(std::thread::id vertex);

public:
	void add_new_expectation(std::thread::id thread_id, std::uint64_t mutex_id);
};

class safe_mutex {
private:
	inline static mutex_graph thread_dependencies;
	inline static std::uint64_t mutex_counter = 0;
	const std::uint64_t this_mutex_id; 
	std::mutex m;
	std::thread_id current_thread;
public:
	safe_mutex();

	void lock();
	void unlock();

	~safe_mutex();
};

}

#endif //SAFE_MUTEX_H