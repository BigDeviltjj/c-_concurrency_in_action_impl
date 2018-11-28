#include<thread>
#include<future>
#include<utility>
#include<iostream>
#include<deque>
#include<string>
#include<vector>
using namespace std;

struct X
{
    void foo(int a, std::string const& str){return;}
    std::string bar(std::string const& str){return str;}
};
X x;
auto f1 = std::async(&X::foo, &x, 42, "hello");
auto f2 = std::async(&X::bar, &x,  "hello");
struct Y
{
    double operator()(double x){return x;}
};
Y y;
auto f3 = std::async(Y(), 3.141);
auto f4 = std::async(std::ref(y), 2.718);
X baz(X& x) { return x;}
auto f6 = std::async(baz, std::ref(x));
class move_only
{
public:
    move_only(){}
    move_only(move_only&& m) {}
    move_only(move_only const&) = delete;
    move_only& operator=(move_only&& m ){return m;}
    move_only& operator=(move_only const&) = delete;
    void operator()(){}
};
auto f5 = std::async(move_only());



int f(int a, string b){
    cout<<a<<" "<<b<<endl;
    for (int i = 0;i<500;++i){
        if (i % 100 == 0)
            cout<<"in f: "<<i<<endl;
    }
}

mutex m;
deque<packaged_task<int(int, string)>> tasks;


template<typename Func, typename... Args>
future<typename result_of<Func(Args&&...)>::type> push_task(Func&& f, Args&&... args) {
//future<int> push_task(function<int(int&, string)> f, int&a, string str){
    typedef typename result_of<Func(Args&&...)>::type result_type;
//    packaged_task<typename remove_reference<Func>::type>  task(forward<Func>(f));
    packaged_task<typename remove_reference<Func>::type>  task(forward<Func>(f));
    future<result_type> res = task.get_future();
    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task));
    auto cur_task = move(tasks.front());
    cur_task(std::forward<Args>(args)...);
    tasks.pop_front();
    return res;
}
int main(){
    int a = 10;
    std::future<int> val = std::async(std::launch::async,f, a, "haha");
    for (int i = 0;i<100000;++i) { i = i+10; i = i  - 10;}
    for (int i = 0;i<1000;++i)
    {
        if (i % 100 == 0)
            cout<<i<<endl;
    }
    cout<<"val is: "<<val.get()<<endl;
    string str = "xixi";
    a = 100;
    auto res = push_task(f, a, move(str));
    cout<<"future get: "<<res.get()<<endl;
}
