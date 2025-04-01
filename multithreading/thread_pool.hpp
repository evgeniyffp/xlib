#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

namespace xlib {
  class thread_pool {
  public:
    thread_pool(size_t numThreads) : stop(false) {
      auto loop_of_work = [this] {
        auto get_task = [this](std::function<void()>& task) {
          std::unique_lock<std::mutex> lock(queueMutex);

          condition.wait(lock, [this] {
            return stop || !tasks.empty();
          });

          if (stop && tasks.empty()) {
            condition.notify_all();
            return false;
          }

          task = std::move(tasks.front());
          tasks.pop();

          return true;
        };

        while (true) {
          std::function<void()> task;
          if (!get_task(task)) {
            break;
          }
          task();
        }
      };

      for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(loop_of_work);
      }
    }

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
      using returnType = typename std::result_of<F(Args...)>::type;

      auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );

      std::future<returnType> res = task->get_future();

      {
        std::unique_lock<std::mutex> lock(queueMutex);

        tasks.emplace([task]() { (*task)(); });
      }

      condition.notify_one();
      return res;
    }

    ~thread_pool() {
      {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
      }

      condition.notify_all();

      for (std::thread& worker : workers)
        worker.join();
    }

  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
  };

  thread_pool global_thread_pool(std::thread::hardware_concurrency());
}

