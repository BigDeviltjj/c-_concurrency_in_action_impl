### C++concurrency in action sample code

C++concurrency in action is a wonderful book which taught me how to write thread safe code and helped me know c++11 more deeply.

However, I found that most of the code in this book is incomplete(only supplies functions or classes) and has some bugs, for example, parentheses are not aligned, Also, the program contains some unsatisfactory designs, e.g., cast std::move rather than std::forward<T> on universal reference

Because of above reasons, I followed the code in the book and rewrite the unreasonable parts.

To run all the code in this repository,

```
g++ ${cpp_file} --std=c++11 -pthread
```

Compling process may fail using visual studio, it is only guaranteed to run successfully with g++4.8 and above.

