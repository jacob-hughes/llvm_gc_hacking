#include <iostream>
#include <stdlib.h>

extern "C" {

    long* gc_alloc(long val) {
        std::cout << "Mallocing: " << val <<std::endl; 
        long *ptr = (long*) malloc(sizeof(long));
        *ptr = val;
        return ptr;
    }

    void logger(long val) {
        std::cout << "Logger: " << val << std::endl;
    }

}
