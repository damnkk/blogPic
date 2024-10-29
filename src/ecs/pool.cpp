#include <ecs/Pool.h>

BasePool::~BasePool(){
    for(char* ptr:blocks_){
        delete [] ptr;
    }
}