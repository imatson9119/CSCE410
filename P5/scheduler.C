/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "linked_list.H"

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  ready_queue = LinkedList<Thread *>();
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  Thread * next = ready_queue.front();
  ready_queue.pop_front();
  if(next != NULL){
     Thread::CurrentThread()->dispatch_to(next);
  }
}

void Scheduler::resume(Thread * _thread) {
  ready_queue.push_back(_thread);
}

void Scheduler::add(Thread * _thread) {
  ready_queue.push_back(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  ready_queue.remove(_thread);
}
