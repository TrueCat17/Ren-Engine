#ifndef THREAD_TASKS_H
#define THREAD_TASKS_H

#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>


class ThreadTasks {
public:
	static ThreadTasks main;
	static ThreadTasks python;


	std::string threadName;
	std::thread::id threadId;

	size_t nextTaskId = 0;

	std::deque<std::pair<const std::function<void()>*, size_t>> queue;
	std::mutex queueMutex;

	std::deque<size_t> calcedIds;
	std::mutex calcedMutex;

	void initForThread();

	void addAndWait(const std::function<void()> &task);


	bool execOne();
	void execAll();                             //call manually (need call initForThread())
	void createThread(const std::string &name); //or auto
};

#endif // THREAD_TASKS_H
