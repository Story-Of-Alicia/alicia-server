#include "server/Scheduler.hpp"

void Scheduler::EnqueueTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    queueCondition.notify_one();
}

void Scheduler::ProcessTasks()
{
    while (true) 
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !taskQueue.empty(); });

            task = std::move(taskQueue.front());
            taskQueue.pop();
        }

        task();
    }
}