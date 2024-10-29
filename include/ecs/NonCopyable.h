#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

class NonCopyAble{
protected:
    NonCopyAble()= default;
    ~NonCopyAble()= default;
    
    NonCopyAble(const NonCopyAble&)= delete;
    NonCopyAble& operator=(const NonCopyAble&)= delete;
};


#endif // NONCOPYABLE_H