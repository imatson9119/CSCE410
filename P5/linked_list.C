/* 
    File: linked_list.C

    Author: Ian Matson
            Department of Computer Science
            Texas A&M University
    Date  : 04/19/21

    Implementation of a linked list.

*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "machine.H"
#include "console.H"

#include "linked_list.H"

/*--------------------------------------------------------------------------*/
/* LOCAL VARIABLES */
/*--------------------------------------------------------------------------*/

/* NONE */

/*--------------------------------------------------------------------------*/
/* L I N K E D   L I S T */
/*--------------------------------------------------------------------------*/
template <typename T>
LinkedList<T>::LinkedList(){
    size = 0;
}

template <typename T>
void LinkedList<T>::push_back(T _data){
    Node<T> * new_node = new Node<T>(_data);
    if(size == 0){
    	head = new_node;
	tail = new_node;
    }
    else{
        new_node->set_next(head);
	head->set_prev(new_node);
	head = new_node;
    }
    size++;
}

template <typename T>
void LinkedList<T>::push_front(T _data){
    Node<T> * new_node = new Node<T>(_data);
    if(size == 0){
    	head = new_node;
	tail = new_node;
    }
    else{
        new_node->set_prev(tail);
	tail->set_next(new_node);
	tail = new_node;
    }
    size++;
}

template <typename T>
void LinkedList<T>::pop_front(){
    if(size != 0){
	Node<T> * t = head;
	head = head->next;
	delete t;
	size--;
    }
}

template <typename T>
void LinkedList<T>::pop_back(){
    if(size != 0){
	Node<T> * t = tail;
	tail = tail->prev;
	delete t;
	size--;
    }
}

template <typename T>
T LinkedList<T>::front(){
    return head->data;
}

template <typename T>
T LinkedList<T>::back(){
    return tail->data;
}

template<typename T>
T LinkedList<T>::get(unsigned int i){
    if(i >= size){
	return (void *) 0;
    }
    Node<T> cur = head;
    for(int j = 0; j < i; j++){
        cur = cur->next;
    }
    return cur->data();
}
