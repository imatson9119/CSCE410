/* 
    File: linked_list.H

    Author: Ian Matson
            Department of Computer Science
            Texas A&M University
    Date  : 4/19/21

    Implementation of a linked list.

*/

#ifndef _LINKED_LIST_H_                   // include file only once
#define _LINKED_LIST_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "node.H"
#include "console.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

template <typename T> class LinkedList {

private:
   unsigned int len;
   Node<T> * head;
   Node<T> * tail;   

public:

   /* Initializes the data structures needed for the management of the 
      linked list. */ 
   LinkedList(){
      len = 0;
      head = NULL;
      tail = NULL;
   }

   /* Pushes item T to the back of the linked list */

   void push_back(T _data){
      Node<T> * new_node = new Node<T>(_data);
      if(len == 0){
         head = new_node;
	 tail = new_node;
      }
      else{
	 new_node->prev = tail;
	 tail->next = new_node;
	 tail = new_node;
      }
      len++;
   }


   /* Pushes item T to the front of the linked list */

   void push_front(T _data){
      Node<T> * new_node = new Node<T>(_data);
      if(len == 0){
    	 head = new_node;
	 tail = new_node;
      }
      else{
         new_node->next = head;
	 head->prev = new_node;
	 head = new_node;
      }
      len++;
   }

   /* Pops an item from the front of the list */
   void pop_front(){
      if(len != 0){
	 if(len == 1){
	    delete head;
	    head = NULL;
	    tail = NULL;
	 } else{
	    Node<T> * t = head;
	    head = head->next;
	    head->prev = NULL;
	    delete t;
	 }
	 len--;
      }
   }

   /* Pops an item from the back of the list */
   void pop_back(){
      if(len != 0){
	 if(size == 1){
	    delete head;
	    head = NULL;
	    tail = NULL;
	 } else{
  	    Node<T> * t = tail;
 	    tail = tail->prev;
	    tail->next = NULL;
	    delete t;
	 }
	 len--;
      }
   }

   /* Returns the data at the front of the list */
   T front(){
      if(head != NULL)
         return head->data;
      return NULL;
   }

   /* Returns the data at the back of the list */
   T back(){
      if(tail != NULL)
         return tail->data;
      return NULL;
   }

   /* gets the data at index i */
   T get(unsigned int i){
      if(i >= len){
 	 return (void *) 0;
      }
      Node<T> cur = head;
      for(int j = 0; j < i; j++){
         cur = cur->next;
      }
      return cur->data();
   }

   /* returns the size of the linked list */
   unsigned int size(){
      return len;
   }

   /* Removes the pointer _node from the linked list */
   void remove(Node<T> * _node){
      if(_node == head && _node == tail){
         head = NULL;
	 tail = NULL;
      } else if(_node == head){
	 head = head->next;
	 head->prev = NULL;
      } else if (_node == tail){
	 tail = tail->prev;
	 tail->next = NULL;
      } else {
      	 _node->prev->next = _node->next;
	 _node->next->prev = _node->prev;
      }
      delete _node;
      len--;
   }

   /* Removes all nodes with value _val from the list */
   void remove(T _val){
      Node<T> * cur = head;
      while(cur != NULL){
         if(cur->data == _val){
	    Node<T> * d = cur;
	    cur = cur->next;
	    remove(d);
	 } else{
            cur = cur->next;
	 }
      }
   }
   /* Prints the current linked list */
   void print(bool print_address = false){
      Node<T> * cur = head;
      while(cur != NULL){
	 if(print_address){
            Console::putui((unsigned long)cur);
	 }
	 else{
            Console::putui((unsigned long)cur->data);
	 }
	 if(cur->next != NULL){
	    Console::puts(" -> ");
	 }	 
	 cur = cur->next;
      }
      Console::puts("\n");
   }
};
#endif
