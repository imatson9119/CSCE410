/* 
    File: node.H

    Author: Ian Matson
            Department of Computer Science
            Texas A&M University
    Date  : 4/19/21

    Implementation of a node used in a linked list.

*/

#ifndef _NODE_H_                   // include file only once
#define _NODE_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* N o d e  */
/*--------------------------------------------------------------------------*/

template <typename T> class Node {

public:
   Node<T> * prev;
   Node<T> * next;
   T data;

   /* Creates an empty instance of the node class */
   Node(){
      prev = NULL;
      next = NULL;
   }

   /* Creates an instance of the node class with populated data */
   Node(T _data){
      data = _data;
      prev = NULL;
      next = NULL;
   }
};
#endif
