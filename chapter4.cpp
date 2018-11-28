#include<iostream>
#include<queue>
#include<thread>
#include<deque>
#include<condition_variable>
#include<memory>
#include<algorithm>
using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue(){}
    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop(){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

template<typename T>
void push_queue(threadsafe_queue<T>& q, T v){
    q.push(v);
}

template<typename T>
void pop_queue(threadsafe_queue<T>& q){
    std::shared_ptr<T> temp = q.wait_and_pop();
    cout<<(*temp)<<endl;
}
int main(){
    threadsafe_queue<int> tq;
    std::vector<std::thread> v;
    for (int i = 0; i < 100;++i){
        v.push_back(std::thread(push_queue<int>, std::ref(tq), i));
        v.push_back(std::thread(pop_queue<int>, std::ref(tq)));
    }
    std::for_each(v.begin(), v.end(), std::mem_fn(&std::thread::join));
    return 0;
}

