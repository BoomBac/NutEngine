#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include "../pch.h"
#include "Log.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>

using std::unique_lock;
using std::make_shared;
using std::packaged_task;
using std::forward;
using std::mutex;
using std::bind;

namespace Engine
{
	class ThreadPool
	{
	public:
		using Task = std::function<void()>;
		ThreadPool()
		{
			Start(std::thread::hardware_concurrency());
		}
		~ThreadPool()
		{
			Stop();
		}
		template<typename Callable, typename...Args>
		auto Enqueue(Callable&& task, Args&&... args)->std::future<std::invoke_result_t<Callable, Args...>>
		{
			using return_type = typename std::invoke_result_t<Callable, Args...>;
			auto wrapper = make_shared<packaged_task<return_type()>>(bind(forward<Callable>(task), std::forward<Args>(args)...));
			std::future<return_type> ret = wrapper->get_future();
			{
				unique_lock<mutex> lock(task_mutex_);
				tasks_.emplace([=]() {
					(*wrapper)();
					});
			}
			task_cv_.notify_one();
			return ret;
		}
	private:
		void Start(size_t num_threads)
		{
			for (auto i = 0u; i < num_threads; ++i)
			{
				threads_.emplace_back([=]() {
					while (true)
					{
						Task task;
						{
							std::unique_lock<std::mutex> lock(task_mutex_);
							task_cv_.wait(lock, [=]() {return stopping_.load() || !tasks_.empty(); });
							if (stopping_.load() && tasks_.empty())
								break;
							task = std::move(tasks_.front());
							tasks_.pop();
						}
						task();
					}
					});
			}
		}
		void Stop() noexcept
		{
			{
				std::unique_lock<std::mutex> lock(task_mutex_);
				stopping_.store(true);
			}
			task_cv_.notify_all();
			for (auto& t : threads_)
				t.join();
		}
	private:
		std::list<std::thread> threads_;
		std::condition_variable task_cv_;
		std::mutex task_mutex_;
		std::atomic<bool> stopping_;
		std::queue<Task> tasks_;
	};
}

#endif // !__THREAD_POOL__

