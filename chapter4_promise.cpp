#include<future>
#include<iostream>
#include<vector>
#include<chrono>
#include<algorithm>
#include<exception>
#include<chrono>

void accumulate( std::vector<int>::iterator first,
                 std::vector<int>::iterator last,
                 std::promise<int> accumulate_promise)
{
    try {
        if (last - first == 1){
            throw std::out_of_range("haha");
        }
        int sum = std::accumulate(first, last, 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        accumulate_promise.set_value(sum);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (std::exception& e) {
        std::cout<<e.what()<<std::endl;
        accumulate_promise.set_exception(std::current_exception());
    }
}

void do_work(std::promise<void> barrier)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier.set_value();
}

int main()
{
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    std::promise<int> accumulate_promise;
    std::future<int> accumulate_future_unique = accumulate_promise.get_future();
//    std::shared_future<int> accumulate_future(std::move(accumulate_future_unique));
    std::shared_future<int> accumulate_future = accumulate_future_unique.share();
    std::cout<<"accumulate_future_unique: "<<accumulate_future_unique.valid()<<std::endl;
    std::cout<<"accumulate_future: "<<accumulate_future.valid()<<std::endl;

    std::thread work_thread(accumulate, numbers.begin(), numbers.end(), std::move(accumulate_promise));

//    accumulate_future.wait();
    if (accumulate_future.wait_for(std::chrono::milliseconds(35)) != std::future_status::ready)
        std::cout<<"not ready!"<<std::endl;
    try {
        std::cout<<"result = "<<accumulate_future.get()<<std::endl;
    } catch (std::exception& e) {
        std::cout<<e.what()<<std::endl;
    }
    work_thread.join();
    std::cout<<" thread 1 end"<<std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto stop = std::chrono::high_resolution_clock::now();
//    std::cout<<" duration: "<<std::chrono::duration<double, std::ratio<1,1000000>>(stop - start).count()<<std::endl;
    std::cout<<" duration: "<<std::chrono::duration<double, std::ratio<1,1>>(stop - start).count()<<std::endl;

    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread new_work_thread(do_work, std::move(barrier));
    barrier_future.wait();
    new_work_thread.join();
    std::cout<<" thread 2 end"<<std::endl;
}

