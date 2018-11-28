#include<atomic>
#include<vector>
#include<thread>
#include<assert.h>
#include<iostream>
#include<chrono>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    x.store(true, std::memory_order_seq_cst);
}
void write_y() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    y.store(true, std::memory_order_seq_cst);
}
void read_x_then_y() {
    while(!x.load(std::memory_order_seq_cst)) std::cout<<"waiting for x"<<std::endl;
    if (y.load(std::memory_order_seq_cst))
        ++z;
}
void read_y_then_x() {
    while(!y.load(std::memory_order_seq_cst)) std::cout<<"waiting for y"<<std::endl;
    if (x.load(std::memory_order_seq_cst))
        ++z;
}
std::vector<int> queue_data;
std::atomic<int> count;

void populate_queue()
{
    unsigned const number_of_items=20;
    queue_data.clear();
    for(unsigned i{0}; i<number_of_items; ++i)
    {
        queue_data.push_back(i);
    }
    count.store(number_of_items, std::memory_order_release);
}
void process(int &x){
    x *= 10;
    std::cout<<x<<std::endl;
}

void consume_queue_item()
{
    while(true)
    {
        int item_index;
        if((item_index=count.fetch_sub(1, std::memory_order_acquire)) <= 0)
        {
            std::cout<<"waiting for data"<<std::endl;
            continue;
        }
        process(queue_data[item_index - 1]);
    }
}
                
void set_count_1() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    count.store(1);
}
void set_count_2() {
    int i = 30;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count.store(i);
        i += 10;
    }
}
void set_count_3() {
    int i = 3;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        count.store(i++);
        if (i > 10) break;
    }
}
void read_count() {
    while(true) {
        if (count.load()) {
            int x = count.load();
            std::cout<<x<<std::endl;
        }
    }
}
int main() {
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x);
    std::thread b(write_y);
    std::thread c(read_x_then_y);
    std::thread d(read_y_then_x);
    a.join();
    b.join();
    c.join();
    d.join();
    assert(z.load()!=0);
    
    std::thread a1(set_count_1);
    std::thread b1(set_count_2);
    std::thread c1(set_count_3);
    std::thread d1(read_count);
    a1.join();
    b1.join();
    c1.join();
    d1.join();

}

