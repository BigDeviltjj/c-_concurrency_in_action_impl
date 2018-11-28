#include<list>
#include<mutex>
#include<thread>
#include<vector>
#include<algorithm>
#include<iostream>
#include<exception>
#include<memory>
#include<stack>
#include<deque>
using namespace std;
template<typename T>
class threadsafe_stack
{
  std::stack<T> data;
  mutable std::mutex m;
public:
  threadsafe_stack(){}
  threadsafe_stack(const threadsafe_stack& other)
  {
    std::lock_guard<std::mutex> lock(other.m);
    data = other.data;
  }

  threadsafe_stack& operator=(const threadsafe_stack&) = delete;
  void push(T new_value){
    std::lock_guard<std::mutex> lock(m);
    data.push(new_value);
  }
    
  std::shared_ptr<T> pop(){
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw "stack empty";
    std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
    data.pop();
    return res;
  }
  void pop(T& value){
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw "stack empty";
    value = data.top();
    data.pop();
  }
  bool empty() const{
    std::lock_guard<std::mutex> lock(m);
    return data.empty();
  }
  friend void swap(threadsafe_stack& lhs, threadsafe_stack& rhs){
    if (&lhs == &rhs)
      return;
//    std::lock(lhs.m, rhs.m);
//    std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
//    std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
    std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
    std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
    std::lock(lock_a, lock_b);
    std::swap(lhs.data, rhs.data);
    lock_a.unlock();
    lock_b.unlock();
  }
};


std::list<int> some_list;
std::mutex some_mutex;
void add_to_list(int new_val)
{
  std::lock_guard<std::mutex> guard(some_mutex);
  some_list.push_back(new_val);
}
bool list_contains(int v2f){
  std::lock_guard<std::mutex> guard(some_mutex);
  return std::find(some_list.begin(), some_list.end(), v2f) != some_list.end();
}

void foo(){
  std::vector<std::thread> v;
  for (int i = 0;i<20;++i)
  {
    v.push_back(std::thread(add_to_list,i));
  }
  std::for_each(v.begin(), v.end(), std::mem_fn(&std::thread::join));
  for (auto &i : some_list){
    cout<<i<<endl;
  }
}
std::once_flag res_flag;
void add_to_stack(threadsafe_stack<int>& s, int val){
  std::lock_guard<std::mutex> guard(some_mutex);
  std::call_once(res_flag, [](){cout<<"i should only show once"<<endl;});
  for (int i = 0;i < 10; ++i){
    s.push(val);
  }
}
void foo1(){

  std::vector<std::thread> v;
  threadsafe_stack<int> s1;
  threadsafe_stack<int> s2;
  for (int i = 0;i<20;++i)
  {
    v.push_back(std::thread(add_to_stack,std::ref(s1), i));
    v.push_back(std::thread(add_to_stack,std::ref(s2), i* 10));
  }
  std::for_each(v.begin(), v.end(), std::mem_fn(&std::thread::join));
  swap(s1,s2);
  while (!s1.empty())
  {
    cout<<*(s1.pop())<<endl;
    cout<<*(s2.pop())<<endl;
  }
  try{
      s1.pop();
  }
  catch( char const * e){
    string s = e;
    cout<<s<<endl;
  }
    
} 
  
int main(){
  foo1();
  return 0;
}
  
