#include<thread>
#include<queue>
#include<exception>
#include<iostream>
#include<mutex>
#include<condition_variable>

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() {}
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }
    
    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() 
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    std::shared_ptr<T> try_pop() 
    {
        std::unique_lock<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
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
void push_data(threadsafe_queue<T> &s, int k)
{
	for (int i = 0; i<100; ++i)
	{
		s.push(i * 2 + k);
	}
}

template<typename T>
void pop_data(threadsafe_queue<T> &s)
{
	while (true) {
		std::cout << "poping " << *(s.wait_and_pop().get()) << std::endl;
	}
}
int main() {
	threadsafe_queue<int> s;

	std::thread t1(push_data<int>, std::ref(s), 1);
	std::thread t2(push_data<int>, std::ref(s), 2);
	t1.join();
	t2.join();
	std::thread p(pop_data<int>, std::ref(s));

	p.join();
	return 0;
}
