#include "threadsafe_queue.h"
#include <iostream>
#include <functional>
#include <atomic>
#include <thread>

class thread_pool
{
    std::atomic_bool done;
    threadsafe_queue<std::function<void()> > worker_queue;
    std::vector<std::thread> threads;
    join_threads joiner;
    void worker_thread()
    {
        while(!done)
        {
            std::function<void()> task;
            if(worker_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:
    thread_pool():
        done(false), joiner(threads)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.push_back(
                        std::thread(&thread_pool::worker_thread, this));
            }
        }
        catch(...)
        {
            done = true;
            throw "an error occured!\n";
        }
    }
    ~thread_pool()
    {
        done = true;
    }
    template<typename FunctionType>
    void submit(FunctionType f)
    {
        worker_queue.push(std::function<void()>(f));
    }
};
std::atomic<int> cnt;
void task() {
    cnt.fetch_add(10);
    std::cout<<cnt.load()<<std::endl;
}
class join_threads
{
    std::vector<std::thread> &threads;
public:
    explicit join_threads(std::vector<std::thread> &threads_):
        threads(threads_)
    {}
    ~join_threads()
    {
        for(unsigned long i = 0; i < threads.size(); ++i)
        {
            if (threads[i].joinable())
                threads[i].join();
        }
    }
};

int main() {
    thread_pool tp;
    for (int i = 0; i < 100; ++i)
    {
        std::function<void()> f = task;
        tp.submit(f);
    }
}
