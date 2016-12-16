#include <iostream>
#include "eventQueue.h"

using namespace std;

struct Node{
  Event* eventPt;
  Node* next;
};
 
  Node* head = '\0';       // let the head pointer point to null

bool enqueue(const Event* e){
  // ******* currently unsure when to return false
  if (e -> time < 0){
    cerr << "Error: time allocated is negative " << endl;
    return false;
  }
  if (head == '\0' || (e -> time) < ((head -> eventPt) -> time)){  // insertion at the head required

    Node* tmp = new Node;
    // added lines of code
    tmp -> eventPt = new Event;
    (tmp -> eventPt) -> type = e -> type;
    (tmp -> eventPt) -> time = e -> time;
    // tmp -> eventPt = e; from previous code
    tmp -> next = head;
    head = tmp;
    // delete tmp; Leave this out for now **********
  }
  else{
    Node * trav1 = head;
	Node * trav2;
    for(; trav1 -> next; trav1 = trav1 -> next){
      trav2 = trav1 -> next;
      if (trav2 == '\0' || (((trav2 -> eventPt) -> time) > (e -> time)))
        break;
    }
	if (trav1 -> next == 0)
		trav2 = trav1 -> next;
    Node* tmp = new Node;
    tmp -> eventPt = new Event;
    (tmp -> eventPt) -> type = e -> type;
    (tmp -> eventPt) -> time = e -> time;
    // tmp -> eventPt = e;
    tmp -> next = trav2;
    trav1 -> next = tmp;
    //delete trav1;
    //delete trav2;
  }
  return true;
}

const Event* dequeue(){
  if (head == '\0'){
    cerr << "Error: there are no nodes to dequeue " << endl;
    return '\0';
  }
  Node* tmp = head;
  head = head -> next;
  // delete tmp;
  return tmp -> eventPt; // Deleting it in the process????
}
