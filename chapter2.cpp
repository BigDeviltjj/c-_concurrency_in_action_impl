#include<iostream>
#include<thread>
#include<string>
using namespace std;
void do_something(const int& i){
    cout<<i<<endl;
}

struct func
{
    int& i;
    func(int& i_): i(i_){}
    void operator()(){
        for (unsigned j = 0; j<100; ++j)
        {
            do_something(j);
        }
    }
};

void oops()
{
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach();
}
struct thread_guard
{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_): t(t_) {}
    ~thread_guard()
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const &) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};
void f(){
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    cout<<"start"<<endl;
}
//    void f()
//    {
//        int some_local_state=0;
//        func my_func(some_local_state);
//        std::thread my_thread(my_func);
//        try
//        {
//            throw "haha";
//        }
//        catch(const char*  msg)
//        {
//            my_thread.join();
//            cerr << msg <<endl;
//        }
//    }
int main()
{
    f();
    cout<<"f out"<<endl;
    return 0;
}
