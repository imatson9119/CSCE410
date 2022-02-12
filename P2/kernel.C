/*
    File: kernel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 02/02/17


    This file has the main entry point to the operating system.

*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
/* Makes things easy to read */

#define KERNEL_POOL_START_FRAME ((2 MB) / (4 KB))
#define KERNEL_POOL_SIZE ((2 MB) / (4 KB))
#define PROCESS_POOL_START_FRAME ((4 MB) / (4 KB))
#define PROCESS_POOL_SIZE ((28 MB) / (4 KB))
/* Definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / (4 KB))
#define MEM_HOLE_SIZE ((1 MB) / (4 KB)) /* We have a 1 MB hole in physical memory starting at address 15 MB */ 
#define TEST_START_ADDR_PROC (4 MB)
#define TEST_START_ADDR_KERNEL (2 MB)
/* Used in the memory test below to generate sequences of memory references. */
/* One is for a sequence of memory references in the kernel space, and the   */
/* other for memory references in the process space. */

#define N_TEST_ALLOCATIONS 
/* Number of recursive allocations that we use to test.  */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"     /* LOW-LEVEL STUFF   */
#include "console.H"

#include "assert.H"
#include "cont_frame_pool.H"  /* The physical memory manager */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void test_memory(ContFramePool * _pool, unsigned int _allocs_to_go);
void exhaustive_memory_test();

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    Console::init();

    /* -- INITIALIZE FRAME POOLS -- */

    /* ---- KERNEL POOL -- */
    
    ContFramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
                                  KERNEL_POOL_SIZE,
                                  0,
                                  0);
    

    /* ---- PROCESS POOL -- */

/*
    unsigned long n_info_frames = ContFramePool::needed_info_frames(PROCESS_POOL_SIZE);

    unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frames(n_info_frames);
    
    ContFramePool process_mem_pool(PROCESS_POOL_START_FRAME,
                                   PROCESS_POOL_SIZE,
                                   process_mem_pool_info_frame,
                                   n_info_frames);
    
    process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);
*/
    /* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

    Console::puts("Hello World!\n");

    /* -- TEST MEMORY ALLOCATOR */
    test_memory(&kernel_mem_pool, 32);
    exhaustive_memory_test();

    /* ---- Add code here to test the frame pool implementation. */
    
    /* -- NOW LOOP FOREVER */
    Console::puts("Testing is DONE. We will do nothing forever\n");
    Console::puts("Feel free to turn off the machine now.\n");

    for(;;);

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
void test_memory(ContFramePool * _pool, unsigned int _allocs_to_go) { Console::puts("alloc_to_go = "); Console::puti(_allocs_to_go); Console::puts("\n");
    if (_allocs_to_go > 0) {
        int n_frames = _allocs_to_go % 4 + 1;
        unsigned long frame = _pool->get_frames(n_frames);
        int * value_array = (int*)(frame * (4 KB));        
        for (int i = 0; i < (1 KB) * n_frames; i++) {
            value_array[i] = _allocs_to_go;
        }
        test_memory(_pool, _allocs_to_go - 1);
        for (int i = 0; i < (1 KB) * n_frames; i++) {
            if(value_array[i] != _allocs_to_go){
                Console::puts("MEMORY TEST FAILED. ERROR IN FRAME POOL\n");
                Console::puts("i ="); Console::puti(i);
                Console::puts("   v = "); Console::puti(value_array[i]); 
                Console::puts("   n ="); Console::puti(_allocs_to_go);
                Console::puts("\n");
                for(;;); 
            }
        }
        ContFramePool::release_frames(frame);
    }
}
void exhaustive_memory_test(){
    Console::puts("Beginning exhaustive memory test\n");
    ContFramePool mem_pool((4 MB) / (4 KB), 1024, 0,0);

    // We have 1023 frames
    unsigned long a = mem_pool.get_frames(13);
    Console::putui(a);
    Console::puts("\n");
    
    unsigned long b = mem_pool.get_frames(10);
    Console::putui(b);
    Console::puts("\n");
    
    unsigned long c = mem_pool.get_frames(1000);
    Console::putui(c);
    Console::puts("\n");

    ContFramePool mem_pool2((8 MB) / (4 KB), 1024,0,0);

    unsigned long d = mem_pool2.get_frames(1000);
    Console::putui(d);
    Console::puts("\n");

    ContFramePool::release_frames(a);
    ContFramePool::release_frames(d);
    ContFramePool::release_frames(b);	
    ContFramePool::release_frames(c);

    unsigned long adds1[1023];

    // Reserve the maximum amount of 1 frame segments
    for(int i = 0; i < 1023; i++){
        adds1[i] = mem_pool.get_frames(1);
    }
    Console::puts("Allocated 1023 1 block segments\n");

    // Free every other frame
    for(int i = 0; i < 1023; i+=2){
       	ContFramePool::release_frames(adds1[i]);
    }
    Console::puts("Deallocated half of segments\n");

    // Reserve back every other frame
    for(int i = 0; i < 1023; i+=2){
	adds1[i] = mem_pool.get_frames(1);
    }
    Console::puts("Reallocated half of segments\n");

    // Free everything
    for(int i = 0; i < 1023; i++){
        ContFramePool::release_frames(adds1[i]);
    }
    Console::puts("Deallocated all 1 block segments\n");

    // Repeat process in 7's
    unsigned long adds2[146];
    for(int i = 0; i < 146; i++){
    	adds2[i] = mem_pool.get_frames(7);
    }
    Console::puts("Allocated 146 7 block segements\n");

    for(int i = 0; i < 146; i+=2){
        ContFramePool::release_frames(adds2[i]);
    }
    Console::puts("Deallocated half of segments\n");
    for(int i = 0; i < 146; i+=2){
   	adds2[i] = mem_pool.get_frames(7);
    }
    Console::puts("Reallocated half of segments\n");
    for(int i = 0; i < 146; i++){
    	ContFramePool::release_frames(adds2[i]);
    }
    Console::puts("Deallocated all 7 block segments\n");
    
    
    Console::puts("Successfully completed exhaustive memory test\n");
}
