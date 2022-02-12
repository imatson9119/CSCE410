/* 
    File: node.C

    Author: Ian Matson
            Department of Computer Science
            Texas A&M University
    Date  : 04/19/21

    Implementation of a node used in a linked list.

*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "machine.H"
#include "console.H"

#include "node.H"

/*--------------------------------------------------------------------------*/
/* LOCAL VARIABLES */
/*--------------------------------------------------------------------------*/

/* NONE */

/*--------------------------------------------------------------------------*/
/* N o d e */
/*--------------------------------------------------------------------------*/

template <typename T>
Node<T>::Node(){
   hasNext = false;
   hasPrev = false;
}

template <typename T>
Node<T>::Node(T _data){
   data = _data;
   hasNext = false;
   hasPrev = false;
}

template <typename T>
void Node<T>::set_next(Node<T> * _next){
   hasNext = true;
   next = _next;
}

template <typename T>
void Node<T>::set_prev(Node<T> * _prev){
   hasPrev = true;
   prev = _prev;
}
