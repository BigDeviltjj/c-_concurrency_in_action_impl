#include <iostream>
#include <mutex>
#include <vector>
#include <numeric>
#include <thread>
#include <algorithm>
#include <future>
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
std::mutex m;
template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return ;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0?hardware_threads:2,max_threads);
    unsigned long const block_size=length/num_threads;
    std::vector<std::future<void>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);
    join_threads joiner(threads);
    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<void(void)> task([=]()
                {
                    std::lock_guard<std::mutex> lock(m);
                    std::for_each(block_start, block_end, f);
                });
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        block_start = block_end;
    }
    std::for_each(block_start,last,f);
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        futures[i].get();
    }
}


template<typename Iterator, typename T>
struct accumulate_block
{
    T operator()(Iterator first, Iterator last)
    {
        return std::accumulate(first, last, T());
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0?hardware_threads : 2, max_threads);
    unsigned long const block_size = length/num_threads;
    std::vector<std::future<T>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        accumulate_block<Iterator, T> tmp = accumulate_block<Iterator, T>();
        std::packaged_task<T(Iterator, Iterator)> task(
                tmp);
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task), block_start, block_end);
        block_start = block_end;
    }
    T last_result = accumulate_block<Iterator, T>()(block_start, last);
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    T result = init;
    for(unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}


int main() {
    std::vector<int> a(500,1);
    int i = 0;
    std::iota(a.begin(), a.end(), 0);
    parallel_for_each(a.begin(), a.end(), [](int const &i){ std::cout<<i<<std::endl;});

    std::cout<<parallel_accumulate(a.begin(), a.end(), 0)<<std::endl;
}

