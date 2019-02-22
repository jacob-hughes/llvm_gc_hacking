#include <iostream>
#include <stdlib.h>
#include "includes/statepoint.h"

#ifdef __linux__
  #define STACKMAP __LLVM_StackMaps
#endif

extern uint8_t STACKMAP[];
bool tableBuilt = false;
statepoint_table_t* table;

int HEAPSIZE = 10 * sizeof(long);
long* FROMSPACE = (long*) malloc(HEAPSIZE);
long* TOSPACE = (long*) malloc(HEAPSIZE);

extern "C" {
    extern long* entry_point(long *x);

    extern void force_gc();

    long* gc_alloc(long val) {
        std::cout << "Mallocing in fromspace: " << val <<std::endl; 
        long *ptr = FROMSPACE;
        *ptr = val;
        FROMSPACE += sizeof(long);
        return ptr;
    }

    long* heapSwp(long *ptr) {
        long val = *ptr;
        *TOSPACE = val;
        std::cout << "Old Fromspace Ptr: " << ptr << std::endl;
        long* newPtr = TOSPACE;
        // zero the memory at the old ptr
        *ptr = 0;
        TOSPACE += sizeof(long);
        std::cout << "New Ptr in tospace: " << newPtr << std::endl;
        return newPtr;
    }

    void logger(long *val) {
        std::cout << "Logger: " << *val << std::endl;
    }

    void walkStack(uint8_t* stackPtr) {
        void* stackmap = (void*)&STACKMAP;
        if(!tableBuilt) {
            printf("printing the table...\n");
            table = generate_table(stackmap, 0.5);
            print_table(stdout, table, true);
            printf("\n\n");
            // destroy_table(table);
            tableBuilt = true;
        }

        std::cout << "Beginning GC" << std::endl;
        uint64_t retAddr = *((uint64_t*)stackPtr);
        printf("Got RA: 0x%" PRIX64 "\n", retAddr);
        stackPtr += sizeof(void*); // step into frame
        frame_info_t* frame = lookup_return_address(table, retAddr);
        while(frame != NULL) {
            std::cout << "Entered Loop" << std::endl;

            uint16_t i;
            for(i = 0; i < frame->numSlots; i++) {
                pointer_slot_t ptrSlot = frame->slots[i];
                if(ptrSlot.kind >= 0) {
                    // our example does not use derived pointers
                    assert(false && "unexpected derived pointer\n");
                }

                uint64_t** ptr = (uint64_t**)(stackPtr + ptrSlot.offset);
                printf("Found Ptr: 0x%" PRIX64 "\n", ptr);
                printf("Performing GC update...\n");
                uint64_t* newPtr = (uint64_t*) heapSwp((long*) *ptr);
                *ptr = newPtr;
                printf("Quick Access of new Ptr: %d \n", **ptr);
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
        long *val = entry_point(gc_alloc(666));
        std::cout << "Derefing the arg to entry_point: " << *val << std::endl;
        return 0;
    }
}
