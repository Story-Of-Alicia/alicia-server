#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

class Scheduler
{
public:
    // Adds a task to the queue
    void EnqueueTask(std::function<void()> task);

    // Starts processing tasks
    void ProcessTasks();

private:
    std::queue<std::function<void()>> taskQueue; // Task queue to hold lambdas
    std::mutex queueMutex;                      // Mutex for queue access
    std::condition_variable queueCondition;     // For notifying the processing thread
};

#endif // SCHEDULER_HPP
