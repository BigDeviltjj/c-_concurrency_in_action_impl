#include<exception>
#include<mutex>
#include<utility>
#include<thread>
#include<stack>
#include<iostream>
struct empty_stack : std::exception
{
	virtual const char* what() const noexcept{
		return "stack empty!";
	}
};
template<typename T>
class threadsafe_stack
{
private:
	std::stack<T> data;
	mutable std::mutex m;
public:
	threadsafe_stack() {}
	threadsafe_stack(const threadsafe_stack &other) {
		std::lock_guard<std::mutex> lock(other.m);
		data = other.data;
	}

	threadsafe_stack &operator=(const threadsafe_stack&) = delete;
	void push(T new_value)
	{
		std::lock_guard<std::mutex> lock(m);
		data.push(std::move(new_value));
	}

	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) throw empty_stack();
		std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
		data.pop();
		return res;
	}
	void pop(T &value) {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) throw empty_stack();
		value = std::move(data.top());
		data.pop();
	}
	bool empty() const {
		std::lock_guard<std::mutex> lock(m);
		return data.empty();
	}
};


template<typename T>
void push_data(threadsafe_stack<T> &s, int k)
{
	for (int i = 0; i<100; ++i)
	{
		s.push(i * k);
	}
}

template<typename T>
void pop_data(threadsafe_stack<T> &s)
{
	while (true) {
		std::cout << "poping " << *(s.pop().get()) << std::endl;
	}
}
int main() {
	threadsafe_stack<int> s;

	std::thread t1(push_data<int>, std::ref(s), 1);
	std::thread t2(push_data<int>, std::ref(s), 2);
	std::thread p(pop_data<int>, std::ref(s));

	t1.join();
	t2.join();
	p.join();
	return 0;
}
