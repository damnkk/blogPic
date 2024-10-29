#ifndef POOL_H
#define POOL_H
#include <vector>
#include <cassert>
class BasePool{
public:
    explicit BasePool(std::size_t element_size,std::size_t chunk_size):element_size_(element_size),chunk_size_(chunk_size),capacity_(0){}
    virtual ~BasePool();

    std::size_t size() const{return size_;}
    std::size_t capacity()const {return capacity_;}
    std::size_t chunkis()const {return chunk_size_;}

    //简单理解成池子扩容吧
    inline void expand(std::size_t n){
        //如果大于size就得更新size,
        if(n>=size_){
            //如果比容量还大就得更新容量
            if(n>=capacity_) reserve(n);
            size_ = n; 
        }
    }
    inline void reserve(std::size_t n){
        while(capacity_<n){
            //内存都是极大块分配,一块一块,8k个8k个的分配
            char* chunk = new char[element_size_*chunk_size_];
            blocks_.push_back(chunk);
            capacity_ += chunk_size_;
        }
    }
    //这不是取多少个,是根据索引取元素之镇
    inline void* get(std::size_t n){
        assert(n<size_);
        return blocks_[n/chunk_size_]+(n%chunk_size_)*element_size_;
    }

    inline const void* get(std::size_t n)const {
        assert(n<size_);
        return blocks_[n/chunk_size_]+(n%chunk_size_)*element_size_;
    }

    virtual void destroy(std::size_t n ) = 0;
protected:
    std::vector<char*> blocks_;
    std::size_t element_size_;
    //一个大块中有多少个元素个数
    std::size_t chunk_size_;
    std::size_t size_ = 0;
    std::size_t capacity_;
};

template<typename T, std::size_t ChunkSize = 8192>
class Pool :public BasePool{
public:
    Pool():BasePool(sizeof(T),ChunkSize){}
    virtual ~Pool(){}

    virtual void destroy(std::size_t n) override{
        assert(n<size_);
        T* ptr = static_cast<T*>(get(n));
        ptr->~T();
    }
};
#endif // POOL_H