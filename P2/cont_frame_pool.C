/*
 File: ContFramePool.C
 
 Author: Ian Matson
 Date  : 2/3/21
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.

 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

unsigned int ContFramePool::npools = 0;
ContFramePool* ContFramePool::pools[8]; 

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // Initializing data members
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;

    // Add the instance reference to the static member pools
    ContFramePool::pools[ContFramePool::npools] = this;
    ContFramePool::npools++;

    // Sets the address of the bitmap (start of the pool if the location is
    // unspecified
    if(info_frame_no == 0){
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);	
    } else {
	bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    if(nframes % 4 != 0){
	Console::puts("WARNING: Number of allocated frames was not divisible by 4, rounded up\n");
	nframes += 4 - (nframes % 4);
    }

    // Set all of the inittial frames to be free
    for(int i = 0; i*4 < nframes; i++){
	bitmap[i] = 0xFF;
    }

    if(info_frame_no == 0){
	// First frame will always be used
	// 10 represents a head of allocated segment, 11, is free, 00 is allocated but not head
	// Set first bit to 1011 1111
	bitmap[0] = 0xBF;

	// If there are any remainining info frames, set them to allocated (00)
	for(int i = 0; i*4 < n_info_frames; i++){
	    for(int j = 1; j < 4*i + j < n_info_frames && j < 4; j++){
		// Sets both bits to 0
		bitmap[0] &= ~(3 << (7-2*j-1) ); 	
	    }
	}
    } else{
	// Do the provided info frames have enought space to store management info?
	assert(n_info_frames * FRAME_SIZE * 4 >= nframes); 
    }
    Console::puts("Frame pool initialized\n");
}


unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // Do we have any frames left?
    assert(nFreeFrames > 0);

    unsigned int frame_no = base_frame_no;
    bool success = false;
    
    unsigned int count = 0;

    // Iterate through frames to find _n_frames consecutive free ones
    for(unsigned int i = 0; i < nframes; i++){
	unsigned int c = i/4;
	unsigned int b = i%4;
	//Console::puts("Currently checking frame ");
	//Console::putui(i);Console::puts(" c:");Console::putui(c);Console::puts(" b:");Console::putui(b);Console::puts("\n");
	// If all of the next 4 frames are used
	if(bitmap[c] == 0x0){
	    //Console::puts("Entire character empty-- skipping ahead\n");
	    count = 0;
	    i+= 3;
	}
	// If the current frame is empty
	else if(bitmap[c] & (0x80 >> b * 2) && bitmap[c] & (0x80 >> b * 2 + 1)){
	    count++;
	    // If count = _n_frames, then we've found a consecutive sequence long enough
	    if(count == _n_frames){
		success = true;
		frame_no = base_frame_no + i - count + 1;
		break;
	    }
	}
    }
    // Check to see if a valid frame was found
    if(!success){
        return 0;
    }

    nFreeFrames -= _n_frames;
    unsigned int start = frame_no - base_frame_no;

    // Set the first frame of allocated space to 10
    bitmap[start/4] |= 0x80 >> start % 4 * 2;
    bitmap[start/4] &= ~(0x80 >> start % 4 * 2 + 1);
   
    // Set the remaining frames to allocated (00)
    for(unsigned int i = start + 1; i < start + _n_frames; i++){
	bitmap[i/4] &= ~(0xC0 >> i % 4 * 2);
    }
    return frame_no;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    assert(_base_frame_no >= base_frame_no && _base_frame_no + _n_frames <= base_frame_no + nframes);
    // The statement is in bounds, mark the first frame as head of sequence
    unsigned long i = _base_frame_no - base_frame_no;

    // Check to see if frame is actually free
    assert(bitmap[i/4] & (0x80 >> i%4 * 2 + 1));

    // Mark inaccessible frame (01)
    bitmap[i/4] &= ~(0x80 >> i%4 * 2);
    bitmap[i/4] |= (0x80 >> i%4 * 2 + 1);
    nFreeFrames--;
    i++;

    // Set the remaining frames to allocated (00)
    for(i; i < _base_frame_no-base_frame_no+nframes; i++){
	// Is the frame actually free?
	assert(bitmap[i/4] & (0x80 >> i%4 * 2 +1));
	bitmap[i/4] &= ~(0xC0 >> i % 4 * 2);
	nFreeFrames--;
    }
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    ContFramePool * ref;
    bool success = false;
    for(int i = 0; i < ContFramePool::npools; i++){
	ContFramePool * cur = ContFramePool::pools[i];
    	if(_first_frame_no >= cur->base_frame_no && _first_frame_no < cur->base_frame_no + cur->nframes){
	    ref = cur;
	    success = true;
	    break;
	}	
    } 
    if(success)
	ref->release_frame(_first_frame_no);
}

void ContFramePool::release_frame(unsigned long _base_frame_no){
    //Console::puts("I am being asked to release frame "); Console::putui(_base_frame_no); Console::puts(" When I have base frame number "); Console::putui(base_frame_no);
    // Are we in bounds?
    assert(_base_frame_no >= base_frame_no && _base_frame_no < base_frame_no + nframes);
    
    unsigned long i = _base_frame_no - base_frame_no;

    //Console::putui(bitmap[i/4]);
    // Is this frame actually a head of frame?
    assert(bitmap[i/4] & (0x80 >> i%4*2) && !(bitmap[i/4] & (0x80 >> i%4*2+1)));
    
    // Set the first frame to free
    bitmap[i/4] |= (0xC0 >> i%4*2);
    i++;
    nFreeFrames++;
    // Iterate until either hof (10) or free frame (11) is encountered, set frames to free (11)
    for(i; i < nframes; i++){
	// If the current frame is either hof or free already
    	if(bitmap[i/4] & (0x80 >> i%4*2) || bitmap[i/4] & (0x80 >> i%4*2+1)) {
	   break;
	}

	// FREE
	bitmap[i/4] |= (0xC0 >> i%4*2);
    	nFreeFrames++;
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return  (_n_frames-1) / (FRAME_SIZE * 4.0) + 1;
}
