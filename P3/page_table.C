#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

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
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   
   page_directory = (unsigned long *)(4 KB * kernel_mem_pool->get_frames(1)); 
   unsigned long * page_table = (unsigned long *)(4 KB * kernel_mem_pool->get_frames(1));
   Console::puts("\nPage table 1 addr1: ");Console::putui((unsigned long)page_table);
   
   // filling in the first page table
   unsigned long address = 0;
   for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
      page_table[i] = address | 3; // supervisor, r/w, present
      address += PAGE_SIZE; // 4kb
   }
   // filling out the first page-directory entry
   page_directory[0] = (unsigned long)page_table;
   page_directory[0] = page_directory[0] | 3; // supervisor, r/w, present
   
   // filling out remaining empty entries
   for(unsigned int i = 1; i < ENTRIES_PER_PAGE; i++){
      page_directory[i] = 0 | 2; // supervisor r/w, not present
   }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   write_cr3((unsigned long)page_directory);
   current_page_table = this;
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   // Retrieve the fault address
   unsigned long fault_addr = read_cr2();
   unsigned long * cur_directory = PageTable::current_page_table->page_directory;

   // Get the index of the desired page_table inside the directory
   unsigned long page_table_index = fault_addr >> 22;
   // Get the index of the desired page inside the page_table
   unsigned long page_index = (fault_addr >> 12)  & ~(0xFFFFFC00);
    
   // If the page_table entry is not not present
   if(!(cur_directory[page_table_index] & 1)){
      // get a frame for the new page table
      unsigned long * page_table = (unsigned long *)(4 KB * kernel_mem_pool->get_frames(1)); 
      
      // Put the address of the new page table in the page directory entry
      cur_directory[page_table_index] = (unsigned long)page_table;
      cur_directory[page_table_index] = cur_directory[page_table_index] | 3; // supervisor, r/w, present
      
      // Initialize the page table
      for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
         page_table[i] = 2; // supervisor, r/w, not present
      }
   }
   
   // be sure to remove info bits 
   unsigned long * page_table = (unsigned long *)(cur_directory[page_table_index] & ~(0xFFF));
   // if page is not present
   if(!(page_table[page_index] & 1)){
      unsigned long  page = 4 KB * process_mem_pool->get_frames(1);
      page = page | 3; // supervisor, r/w, present
      page_table[page_index] = page; // Put it in the page table!
   }
   // Console::puts("handled page fault\n");
}

