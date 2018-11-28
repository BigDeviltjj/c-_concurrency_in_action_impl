#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <algorithm>

#include "threadsafe_queue.h"

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

class function_wrapper
{
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type: impl_base
    {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call() {f();}
    };
public:
    template<typename F>
    function_wrapper(F&& f):
        impl(new impl_type<F>(std::forward<F>(f)))
    {}
    void operator()()
    {
        impl->call();
    }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other):
        impl(std::move(other.impl))
    {}
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

class thread_pool
{
    std::atomic_bool done;
    std::vector<std::thread> threads;
    join_threads joiner;
    threadsafe_queue<function_wrapper> worker_queue;

    void worker_thread()
    {
        while (!done)
        {
            function_wrapper task;
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
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        worker_queue.push(std::move(task));
        return res;
    }
};

std::atomic<int> cnt;
int f()
{
    int x = cnt.fetch_add(10);
    return x;
}


int main() {
    thread_pool tp;
    std::vector<std::future<int>> futures;
    for (int i = 0;i < 10;++i)
    {
        futures.push_back(std::move(tp.submit(f)));
    }
    std::for_each(futures.begin(), futures.end(), [] (std::future<int> &f) {std::cout<<f.get()<<std::endl;});
}

