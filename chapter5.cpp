#include<iostream>
#include<algorithm>
#include<vector>
#include<iostream>
#include<thread>
#include<future>
#include<atomic>
#include<chrono>

class spinlock_mutex
{
    std::atomic_flag flag;
public:
    spinlock_mutex(): flag(ATOMIC_FLAG_INIT)
    {}
    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }
    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};
spinlock_mutex m;
int f(int x) {
    m.lock();
    std::cout<<"1. In f: "<<x<<std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout<<"2. In f: "<<x<<std::endl;
    m.unlock();
    return x;
}

std::vector<int> data;
std::atomic<bool> data_ready(false);

void reader_thread() {
    while(!data_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout<<"Answer= "<<data[0]<<"\n";
}

void writer_thread() {
    std::cout<<"writer thread"<<std::endl;
    data.push_back(42);
    data_ready = true;
}
int main() {
    std::vector<std::future<int>> v;
    for (int i{0}; i < 10; ++i) {
        v.push_back(std::async(std::launch::async,f, i));
    }
    std::for_each(v.begin(), v.end(), [](std::future<int> &t) {std::cout<<t.get()<<std::endl;});
    std::thread t2 = std::thread(reader_thread);
    std::thread t1 = std::thread(writer_thread);
    t1.join();
    t2.join();

}
