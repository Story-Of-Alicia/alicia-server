#include "libserver/Executor.hpp"

#include "spdlog/spdlog.h"

#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <thread>

namespace server {

// holds the task_executor parameters
struct task_executor_param
{
  std::function<void()>   task;
  std::condition_variable *cv;
};

// pthread start_routine function
void * task_executor(void * param)
{
  // make the thread cancellable
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
  // make the thread stop execution anywhere
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

  // cast parameters to the correct struct
  const auto p = static_cast<struct task_executor_param*>(param);

  p->task();
  p->cv->notify_one();

  return nullptr;
}

Executor::Executor(uint64_t const timeout) {
  _executor = std::thread([this, timeout]{
    while (true)
    {
      // await internal change
      _notification.wait(false);

      std::function<void()> task;
      {
        // lock queue access mutex
        std::scoped_lock lock(_queueMutex);

        if (_queue.empty() && !_shouldRun)
        {
          break; // if the queue is empty, and the executor shouldn't run, it's a good time to exit
        }

        if (_queue.empty())
        {
          _notification = false; // this will await changes
          continue;
        }

        task = _queue.front();
        _queue.pop();
      }

      std::condition_variable condition;

      task_executor_param param = {
        .task = task,
        .cv   = &condition,
      };

      pthread_t taskThread;
      if (pthread_create(&taskThread, nullptr, &task_executor, &param) != 0)
      {
        spdlog::error("pthread_create failed: {}", strerror(errno));
        continue;
      }

      if (timeout == 0)
        return;

      std::mutex mutex;
      std::unique_lock lock(mutex);

      // wait for timeout milliseconds before terminating the execution thread
      if (condition.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout)
      {
        if (pthread_cancel(taskThread) != 0)
        {
          spdlog::error("pthread_cancel failed: {}", strerror(errno));
          exit(0XDEAD);
        }

        void * ret;

        // the only way to know if the thread was cancelled it to join with it, and compare the return value to PTHREAD_CANCELED
        if (pthread_join(taskThread, &ret) != 0)
        {
          spdlog::error("pthread_join failed: {}", strerror(errno));
          exit(0XDEAD);
        }

        if (ret == PTHREAD_CANCELED)
        {
          spdlog::error("task was cancelled, timeout reached");
          //free(ret);
          // i'm pretty sure free needs to be called, but this throws sigsegv
          exit(0xDEAD);
        }

        //free(&ret);
      }
    }
  });
}

Executor::~Executor() {
  _shouldRun = false;

  // notify of internal change (_shouldRun changed)
  _notification = true;
  _notification.notify_one();

  _executor.join();
}

void Executor::execute(std::function<void()>const & function)
{
  std::lock_guard lock(_queueMutex);

  if (!_shouldRun)
  {
    //TODO: could lead to issues when exit-related tasks are issued.
    return; // don't accept new tasks when exiting, maybe log?
  }

  _queue.push(function);

  // notify of internal change (_queue updated)
  _notification = true;
  _notification.notify_one();
}

}