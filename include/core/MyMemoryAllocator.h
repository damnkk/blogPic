#ifndef MY_MEMORY_ALLOCATOR_H
#define MY_MEMORY_ALLOCATOR_H
#include <memory>
#include <utility>
#include <vector>
#include <iostream>


template<typename T>
struct MyMemoryAllocator{
   using value_type = T;
    MyMemoryAllocator() = default;

    std::allocator<T> allocator;

    T* allocate(size_t n){
        std::cout<<"allocate"<<std::endl;
        return static_cast<T*>(malloc(n*sizeof(T)));
    };
    void deallocate(T*p,size_t n){
        std::cout<<"deallocate"<<std::endl;
        free(p);
    }
    template<typename ...Args>
    void construct(T*p,Args&&...args){
        std::cout<<"construct"<<std::endl;
       new(p)T(std::forward<Args>(args)...);
    }

    void destroy(T*p){
        std::cout<<"destroy"<<std::endl;
        p->~T();
    }
};


template<typename T>
class Vector:public std::vector<T,MyMemoryAllocator<T>>{

};



#endif // MY_MEMORY_ALLOCATOR_H