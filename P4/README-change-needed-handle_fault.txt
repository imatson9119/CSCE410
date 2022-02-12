On your method PageTable::handle_fault, 
you need add an invocation to check the address. 
A suggestion is to do something like:

   unsigned long addr = read_cr2(); /* you already had something like
                                    ** this at the begining of your
                                    ** implementation for handle_fault 
                                    */

   /* Now the suggested code. It uses output utilities from utils.H 
   ** You put this code after reading the faulting address, and before
   ** the code that is handling the fault.
   */
   if (!current_page_table->check_address(addr)) {
       Console::puts("check_address failed for address: ");
       Console::putui(addr);
       Console::puts(" (this in hexa ");
       char hexastr[9];
       ulong2hexstr(addr, hexastr);
       Console::puts(hexastr);
       Console::puts(")\n");
       abort();
   }

   /* Here goes the rest of your P3 handle_fault
   */