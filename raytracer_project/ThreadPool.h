#pragma once


#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <vector>
#include <memory>

#define MAX_THREADS 8 //testing


// a queue with mutex locks for multithreaded access.

namespace TP
{

template <typename T>
class TaskQueue
{
public:
    bool push(T const& value)
    {
        // unique_lock default constructor locks the mutex 
        // untill destructor is called at the end of the function
        std::unique_lock<std::mutex> lock(this->mutex);
        this->queue.push(value);
        return true;
    }

    bool pop(T& v)
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->queue.empty())
            return false;
        v = this->queue.front();
        this->queue.pop();
        return true;
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        return this->queue.empty();
    }

private:
    std::queue<T> queue;
    std::mutex mutex;
};

// TODO - not used at the moment
template <typename T>
class TaskPriorityQueue
{
public:
    bool push(T const& value)
    {
        // unique_lock default constructor locks the mutex 
        // untill destructor is called at the end of the function
        std::unique_lock<std::mutex> lock(this->mutex);
        this->queue.push(value);
        return true;
    }

    bool pop(T& v)
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->queue.empty())
            return false;
        v = this->queue.front();
        this->queue.pop();
        return true;
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        return this->queue.empty();
    }

private:
    std::priority_queue<T> queue;
    std::mutex mutex;
};


class ThreadPool
{
public:
    ThreadPool() { this->init(); }
    ThreadPool(int nThreads) { this->init(); this->resize(nThreads); }

    ~ThreadPool() { this->stop(true); } // wait until ready

    // number threads
    int size() { return static_cast<int>(this->threads.size()); }
    // number of idle
    int nIdle() { return this->nWaiting; }
    // TODO test this
    int nRun() { return size() - nIdle(); }
    // get i:th thread
    std::thread& getThread(int i) { return *this->threads[i]; }

    // change the size of the thread pool
    void resize(int nThreads);
    void clearQueue();
    std::function<void(int)> pop();
    void stop(bool isWait = false);


    template<typename F, typename... Rest>
    auto push(F&& f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
            );
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
            });
        this->q.push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
        return pck->get_future();
    }

    // run the user's function that excepts argument int - id of the running thread. returned value is templatized
    // operator returns std::future, where the user can get the result and rethrow the catched exceptins
    template<typename F>
    auto push(F&& f) ->std::future<decltype(f(0))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
            });
        this->q.push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
        return pck->get_future();
    }


private:
    ThreadPool(const ThreadPool&);
    ThreadPool(ThreadPool&&);
    ThreadPool& operator=(const ThreadPool&);
    ThreadPool& operator=(ThreadPool&&);

    void setThread(int i);
    void init() { this->nWaiting = 0; this->isStop = false; this->isDone = false; }


    // thread container
    std::vector<std::unique_ptr<std::thread>> threads;
    // flag container
    std::vector<std::shared_ptr<std::atomic<bool>>> flags;
    // queue of functions to run
    TaskQueue<std::function<void(int id)>*> q;

    std::atomic<bool> isDone;
    std::atomic<bool> isStop;
    std::atomic<int> nWaiting; // number of threads waiting

    // lock stuff
    std::mutex mutex;
    std::condition_variable cv;

};


}




