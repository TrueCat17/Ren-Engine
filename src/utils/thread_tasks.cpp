#include "thread_tasks.h"

#include "utils/utils.h"


ThreadTasks ThreadTasks::main;
ThreadTasks ThreadTasks::python;


void ThreadTasks::initForThread() {
	threadId = std::this_thread::get_id();
}


void ThreadTasks::addAndWait(const std::function<void()> &task) {
	std::thread::id curThreadId = std::this_thread::get_id();
	if (threadId == curThreadId) {
		task();
		return;
	}

	size_t taskId;
	{
		std::lock_guard g(queueMutex);
		taskId = ++nextTaskId;
		queue.push_back({ &task, taskId });
	}
	while (true) {
		{
			std::lock_guard g(calcedMutex);
			if (!calcedIds.empty() && calcedIds.front() == taskId) {
				calcedIds.pop_front();
				return;
			}
		}

		bool haveTasks = false;
		if (curThreadId == ThreadTasks::main.threadId) {
			haveTasks = ThreadTasks::main.execOne();
		}else
		if (curThreadId == ThreadTasks::python.threadId) {
			haveTasks = ThreadTasks::python.execOne();
		}

		if (!haveTasks) {
			Utils::sleep(10 * 1e-6);
		}
	}
}


bool ThreadTasks::execOne() {
	const std::function<void()> *task;
	size_t taskId;
	{
		std::lock_guard g(queueMutex);
		if (queue.empty()) return false;

		task   = queue[0].first;
		taskId = queue[0].second;
		queue.pop_front();
	}
	(*task)();

	std::lock_guard g(calcedMutex);
	calcedIds.push_back(taskId);

	return true;
}

void ThreadTasks::execAll() {
	while (execOne()) {}
}

static void thread(ThreadTasks *tasksPtr) {
	Utils::setThreadName(tasksPtr->threadName);
	tasksPtr->threadId = std::this_thread::get_id();

	while (true) {
		tasksPtr->execAll();
		Utils::sleep(50 * 1e-6);
	}
}

void ThreadTasks::createThread(const std::string &name) {
	threadName = name;
	std::thread(thread, this).detach();
}
