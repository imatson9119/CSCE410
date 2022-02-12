#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    // replace the assertion with your P3 implementation for this method
    assert(false);
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    // replace the assertion with your P3 implementation for this method
    assert(false);
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    // replace the assertion with your P3 implementation for this method
    assert(false);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // replace the assertion with your P3 implementation for this method
    assert(false);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    // replace the assertion with your implementation for this method,
    // BUT YOU ALSO NEED TO MAKE A SMALL CHANGE to
    // the method. Look at the file README-change-needed-handle_fault.txt
    // 
    // Your implementation can be either:
    // (a) the one from P3 (i.e., you did not implement the "recursive trick"
    //     that allows for inner page tables to reside in virtual memory)
    // or
    // (b) your new PageTable implementation that supports large address
    //     spaces (i.e., it allocates inner page tables from
    //     the process pool)
    // 
    assert(false);
    Console::puts("handled page fault\n");
}

bool PageTable::check_address(unsigned long address)
{
    // you need to implement this for P4 Part II.
    // It returns true if legitimate, false otherwise.
    // You check for legitimacy using VMPool::is_legitimate,
    // which you will implement for real in P4 Part III.
    // For part II, we give you a fake implementation
    // in vm_pool.C for you to use for  now.
    assert(false);
    return false; // you need to implement this
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    // you need to implement this for P4 Part II.
    assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    // you need to implement this for P4 Part II.
    assert(false);
    Console::puts("freed page\n");
}
