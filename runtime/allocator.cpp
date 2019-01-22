#include <iostream>
#include <stdlib.h>
#include "includes/statepoint.h"

#ifdef __linux__
  #define STACKMAP __LLVM_StackMaps
#endif

extern uint8_t STACKMAP[];
bool tableBuilt = false;
statepoint_table_t* table;


extern "C" {
    int entry_point();

    long* gc_alloc(long val) {
        std::cout << "Mallocing: " << val <<std::endl; 
        long *ptr = (long*) malloc(sizeof(long));
        *ptr = val;
        return ptr;
    }

    void logger(long *val) {
        std::cout << "Logger: " << *val << std::endl;
    }

    void genTable() {
        void* stackmap = (void*)&STACKMAP;
        if(!tableBuilt) {
            printf("printing the table...\n");
            table = generate_table(stackmap, 0.5);
            print_table(stdout, table, true);
            printf("\n\n");
            // destroy_table(table);
            tableBuilt = true;
        }
    }

    void walkStack(uint8_t* stackPtr) {
        std::cout << "Beginning GC" << std::endl;
        uint64_t retAddr = *((uint64_t*)stackPtr);
        stackPtr += sizeof(void*); // step into frame
        frame_info_t* frame = lookup_return_address(table, retAddr);
        while(frame != NULL) {

            uint16_t i;
            for(i = 0; i < frame->numSlots; i++) {
                pointer_slot_t ptrSlot = frame->slots[i];
                if(ptrSlot.kind >= 0) {
                    // our example does not use derived pointers
                    assert(false && "unexpected derived pointer\n");
                }

                uint32_t** ptr = (uint32_t**)(stackPtr + ptrSlot.offset);
                std::cout << "Found Pointer: " << ptr << std::endl;
            }

            // move to next frame. seems we have to add one pointer size to
            // reach the next return address? NOTE
            stackPtr = stackPtr + frame->frameSize;

            // grab return address of the frame
            retAddr = *((uint64_t*)stackPtr);
            stackPtr += sizeof(void*); // step into frame
            frame = lookup_return_address(table, retAddr);

        }

    }

    int main() {
        entry_point();
        return 0;
    }


}
