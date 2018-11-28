#include<iostream>
#include<future>
#include<list>
#include<algorithm>

    
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const &pivot=*result.begin();
    auto divide_pointer = std::partition(input.begin(), input.end(), [&](T const &t){return t<pivot;});

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_pointer);
    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const &pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const &t) {return t<pivot;});
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    std::future<std::list<T>> new_lower(std::async(&parallel_quick_sort<T>, std::move(lower_part)));

    auto new_heigher(
            parallel_quick_sort(std::move(input)));
    result.splice(result.end(), new_heigher);
    result.splice(result.begin(), new_lower.get());
    return result;
}
int main() {
    std::list<int> t{5,3,1,0,8,6,4};
    std::list<int> res = sequential_quick_sort(t);
    std::list<int> res_p = parallel_quick_sort(t);
    std::for_each(res.begin(), res.end(), [](int const &t){ std::cout<<t<<" ";});
    std::cout<<std::endl;
    std::for_each(res_p.begin(), res_p.end(), [](int const &t){ std::cout<<t<<" ";});
    std::cout<<std::endl;
    return 0;
}

