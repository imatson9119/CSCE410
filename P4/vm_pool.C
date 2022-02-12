/*
 File: vm_pool.C
 
 Author: Ian Matson
 Date  : 4/5/21
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    num_allocated = 0;
    assert(!(base_address & 0x000003FF)) // Base address must be a page boundary
    base_address = _base_address;
    size = ((_size - 1) / (4 KB) + 1) * 4 KB; // Round to the nearest frame size
    frame_pool = _frame_pool;
    page_table = _page_table;
    allocated_list = (RegionInfo *)_base_address;
    RegionInfo infoFrame; 
    infoFrame.start_frame = base_address;
    infoFrame.size = 4 KB;

    // Register pool with the page_table
    page_table->register_pool(this);
    
    allocated_list[0] = infoFrame;
    num_allocated = 1; // Set to one because we reserve a spot for the list of allocations
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    _size = ((_size -1) / (4 KB) + 1) * 4 KB; // Round size to the nearest frame
    unsigned int ret_address;
    bool found = false;
    // We don't need to check the space before the first entry in allocated list because
    // it is always used to store the array info itself
    for(unsigned int i = 0; i < num_allocated-1; i++){
	// If there is space between entry i and i+1
    	if(allocated_list[i+1].start_frame - allocated_list[i].start_frame - allocated_list[i].size >= _size){
	    ret_address = allocated_list[i].start_frame + allocated_list[i].size;
	    RegionInfo info;
	    info.start_frame = ret_address;
	    info.size = _size;
	    insert_item(info, i+1);
	    found = true;
	    break;
	}
    }

    // We still need to check if there's space after the last item
    if( !found && base_address + size - allocated_list[num_allocated-1].start_frame 
		    - allocated_list[num_allocated-1].size >= _size){
        // Set the return address to be the first free frame after the last region
	ret_address = allocated_list[num_allocated-1].start_frame + allocated_list[num_allocated-1].size;
        RegionInfo info;
        info.start_frame = ret_address;
        info.size = _size;
        insert_item(info, num_allocated);
	found = true;
    }
    if(found)
        Console::puts("Allocated region of memory.\n");
    else{
	Console::puts("ERROR: FAILED TO ALLOCATE MEMORY");
        assert(false);
    }
    return ret_address;
}

void VMPool::release(unsigned long _start_address) {
    unsigned long region_num = 0;
    // Skip the first entry (contains list info)
    for(unsigned int i = 1; i < num_allocated; i++){
    	// Check to see if the address exists
    	if(allocated_list[i].start_frame == _start_address){
	    region_num == i;
	    break;
	}
    }
    // If region_num = 0, we never found the corresponding region to release
    if(region_num == 0){
        Console::puts("ERROR: attempted to release non-allocated region!\n");
	return;
    }
    
    // We need to free all pages in the region, starting with the page below
    unsigned long page_no = allocated_list[region_num].start_frame / (4 KB);
    unsigned long end_frame = (allocated_list[region_num].start_frame + allocated_list[region_num].size) / (4 KB);
    for(page_no; page_no < end_frame; page_no++){
    	page_table->free_page(page_no);
    }

    // Remove the entry from the allocated region list
    remove_item(region_num);

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    bool valid = false;
    // This means we are trying to reserve a frame for the list itself
    if(num_allocated == 0){
        return true;
    }
    for(unsigned int i = 0; i < num_allocated; i++){
	// Check to see if the address in in bounds of the current region
	if(allocated_list[i].start_frame <= _address 
	   && (allocated_list[i].start_frame + allocated_list[i].size) > _address){
	    valid = true;
	    break;
	}
    }
    return valid;
}

void VMPool::insert_item(RegionInfo x, unsigned int index){
    for(unsigned int i = num_allocated - 1; i >= index; i--){
	// Shift over item by 1
    	allocated_list[i+1] = allocated_list[i];
    }
    // Everything is moved over, so we can insert the item.
    allocated_list[index] = x;
    num_allocated++;
}
void VMPool::remove_item(unsigned int index){
    for(unsigned int i = index; i < num_allocated; i++){
    	// Shift everything over by 1
	allocated_list[i] = allocated_list[i+1];
    }
    // Clean up the previous entry
    allocated_list[num_allocated-1].start_frame = 0;
    allocated_list[num_allocated-1].size = 0;
    num_allocated--;
}
