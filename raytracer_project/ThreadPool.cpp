#include "ThreadPool.h"

using namespace TP;

void ThreadPool::resize(int nThreads)
{
	if (!this->isStop && !isDone)
	{
		int oldNThreads = static_cast<int>(this->threads.size());
		if (oldNThreads <= nThreads) // growing 
		{
			this->threads.resize(nThreads);
			this->flags.resize(nThreads);

			for (int i = oldNThreads; i < nThreads; ++i)
			{
				this->flags[i] = std::make_shared<std::atomic<bool>>(false);
				this->setThread(i);
			}
		}
		else // shrinking
		{
			for (int i = oldNThreads - 1; i >= nThreads; --i)
			{
				// detach the removed threads
				*this->flags[i] = true;
				this->threads[i]->detach();
			}
			{// braces make new scope for the lock to exist only for this
				std::unique_lock<std::mutex> lock(this->mutex);
				this->cv.notify_all();
			}
			this->threads.resize(nThreads);
			this->flags.resize(nThreads);
		}
	}
}


void ThreadPool::clearQueue()
{
	std::function<void(int id)>* _f;
	while (this->q.pop(_f))
		delete _f;
}


std::function<void(int)> ThreadPool::pop()
{
	std::function<void(int id)>* _f = nullptr; // null pointer to a task
	this->q.pop(_f); // remove task, _f is the task now
	std::unique_ptr<std::function<void(int id)>> func(_f); // no clue
	std::function<void(int)> f;
	if (_f)
		f = *_f;
	return f;
}


void ThreadPool::stop(bool isWait)
{
	if (!isWait)
	{
		if (this->isStop)
			return;

		this->isStop = true;
		for (int i = 0, n = this->size(); i < n; i++)
		{
			*this->flags[i] = true;
		}
		this->clearQueue();
	}
	else
	{
		if (this->isDone || this->isStop)
			return;
		this->isDone = true;
	}
	{
		std::unique_lock<std::mutex> lock(this->mutex);
		this->cv.notify_all();
	}
	for (int i = 0; i < static_cast<int>(this->threads.size()); ++i)
	{
		if (this->threads[i]->joinable())
			this->threads[i]->join();
	}
	this->clearQueue();
	this->threads.clear();
	this->flags.clear();
}


void ThreadPool::setThread(int i)
{
	std::shared_ptr<std::atomic<bool>> flag(this->flags[i]);
	auto f = [this, i, flag]() {
		std::atomic<bool>& _flag = *flag;
		std::function<void(int id)>* _f;
		bool isPop = this->q.pop(_f);
		while (true)
		{
			while (isPop)
			{
				std::unique_ptr<std::function<void(int id)>> func(_f);
				(*_f)(i);
				if (_flag)
					return;
				else
					isPop = this->q.pop(_f);
			}
			std::unique_lock<std::mutex> lock(this->mutex);
			this->nWaiting++;
			this->cv.wait(lock, [this, &_f, &isPop, &_flag]() {
				isPop = this->q.pop(_f); return isPop || this->isDone || _flag; });
			this->nWaiting--;
			if (!isPop)
				return;
		}
	};
	this->threads[i].reset(new std::thread(f));
}
