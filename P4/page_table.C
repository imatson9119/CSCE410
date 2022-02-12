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
   num_vmPools = 0;
   
   page_directory = (unsigned long *)(4 KB * process_mem_pool->get_frames(1)); 
   unsigned long * page_table = (unsigned long *)(4 KB * process_mem_pool->get_frames(1));
   
   // filling in the first page table
   unsigned long address = 0;
   for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
      page_table[i] = address | 3; // supervisor, r/w, present
      address += PAGE_SIZE; // 4kb
   }
   // filling out the first page-directory entry
   page_directory[0] = (unsigned long)page_table;
   page_directory[0] = page_directory[0] | 3; // supervisor, r/w, present
   
   // Set the last entry in the page_directory to point to itself
   page_directory[ENTRIES_PER_PAGE-1] = (unsigned long)page_directory | 3; // supervisor, r/w, present

   // filling out remaining empty entries
   for(unsigned int i = 1; i < ENTRIES_PER_PAGE-1; i++){
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

   // Check to see if address is valid
   if(!current_page_table->check_address(fault_addr)){
      Console::puts("check_address failed for address: ");
      Console::putui(fault_addr);
      Console::puts(" (this in hexa ");
      char hexastr[9];
      ulong2hexstr(fault_addr,hexastr);
      Console::puts(hexastr);
      Console::puts(")\n");
      abort();
   }

   // Get the index of the desired page_table inside the directory
   unsigned long page_table_index = get_first_10_bits(fault_addr);
   // Get the index of the desired page inside the page_table
   unsigned long page_index = get_middle_10_bits(fault_addr);
   
   // Construct the address used to access the page directory entry
   unsigned long * pde_addr = construct_pde_address(page_table_index);

   // If the page_table entry is not not present
   if(!(*pde_addr & 1)){
      // get a frame for the new page table
      unsigned long * pte_physical_addr = (unsigned long *)(4 KB * process_mem_pool->get_frames(1)); 
      
      // Put the address of the new page table in the page directory entry
      *pde_addr = (unsigned long)pte_physical_addr | 3; // supervisor, r/w, present
      
      // Initialize the page table
      for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
         *construct_pte_address(page_table_index, i) = 2; // supervisor, r/w, not present
      }
   }
   
   // be sure to remove info bits 
   unsigned long * pte_addr = construct_pte_address(page_table_index, page_index);
   // if page is not present
   if(!(*pte_addr & 1)){
      unsigned long  page = 4 KB * process_mem_pool->get_frames(1);
      page = page | 3; // supervisor, r/w, present
      *pte_addr = page; // Put it in the page table!
   }
   // Console::puts("handled page fault\n");
}

unsigned long PageTable::get_middle_10_bits(unsigned long a){
    return (a >> 12)  & ~(0xFFFFFC00);
}

unsigned long PageTable::get_first_10_bits(unsigned long a){
   return a >> 22;
}

unsigned long * PageTable::construct_pte_address(unsigned long pdi, unsigned long pti){
   // First 10 bits set to 1
   unsigned long addr = 0xFFC00000;

   // Add in the index for the page table and directory
   addr = addr | pdi << 12;
   addr = addr | pti << 2;

   return (unsigned long *) addr;
}

unsigned long * PageTable::construct_pde_address(unsigned long pdi){
   // First 20 bits set to 1
   unsigned long addr = 0xFFFFF000;

   // Add in the index for the page directory
   addr = addr | pdi << 2;

   return (unsigned long *) addr;
}


bool PageTable::check_address(unsigned long address)
{
   //Console::puts("In check_address, there are ");Console::putui(num_vmPools);Console::puts(" vm pools\n");
   bool valid = false;
   for(unsigned int i = 0; i < num_vmPools; i++){
      if(vmPools[i]->is_legitimate(address)){
         valid = true;
      }
   }
   return valid;
}

void PageTable::register_pool(VMPool * _vm_pool){
   vmPools[num_vmPools] = _vm_pool;
   num_vmPools++;
   Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
   unsigned long page_phys_addr = _page_no * PAGE_SIZE;
   unsigned long dir_entry_index = _page_no >> 10;
   unsigned long pt_entry_index = _page_no & ~(0xFFFFFC00);
    
   unsigned long * pde_addr = construct_pde_address(dir_entry_index);
   unsigned long * pte_addr = construct_pte_address(dir_entry_index,pt_entry_index);

   // If both the directory entry and page table entry are valid
   if(*pde_addr & 1 && *pte_addr & 1){
      // Make sure to clear the info bits
      unsigned long frame_addr = *pte_addr & ~(0x3FF);

      this->process_mem_pool->release_frames(frame_addr / PAGE_SIZE);

      // Mark as no longer present
      *pte_addr = 2;

      // Reload the TLB
      write_cr3(read_cr3());
      Console::puts("freed page\n");
   } else{
      Console::puts("WARNING: ATTEMPTED TO FREE UNUSED PAGE\n");
   }
}
