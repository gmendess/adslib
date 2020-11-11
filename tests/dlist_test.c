#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../src/dlist/dlist.h"


static inline void ads_dlist_add_TEST(void) {
  ads_dlist_t dlist = {0};
  ads_dlist_init(&dlist, NULL);

  int t[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
  ads_dlist_node_t* node = NULL;

  // check list size
  assert(ads_dlist_get_size(&dlist) == 0);

  // add at head
  assert(!ads_dlist_add_front(&dlist, &t[0]));

  // check if head and tail points to the same node
  assert(ads_dlist_get_head(&dlist) == ads_dlist_get_tail(&dlist));

  // add at tail
  assert(!ads_dlist_add_back(&dlist, &t[1]));

  // check if head and tail points to the same node
  assert(ads_dlist_get_head(&dlist) != ads_dlist_get_tail(&dlist));

  // get tail
  node = ads_dlist_get_tail(&dlist);

  // add at tail using `node`
  assert(!ads_dlist_add_next(&dlist, node, &t[2]));
  
  // add a previous node to the one that used to be the tail
  assert(!ads_dlist_add_prev(&dlist, node, &t[3]));

  // get head
  node = ads_dlist_get_head(&dlist);

  // add after the head using `node`
  assert(!ads_dlist_add_next(&dlist, node, &t[4]));
  
  // add at head using `node`
  assert(!ads_dlist_add_prev(&dlist, node, &t[5]));

  // add at head
  assert(!ads_dlist_add_front(&dlist, &t[6]));

  // add at tail
  assert(!ads_dlist_add_back(&dlist, &t[7]));

  /* ======= at this point, the list must be [70, 60, 10, 50, 40, 20, 30, 80] ======= */

    // check position 0 (head)
  assert( *((int*) dlist.head->data) == 70);

  // check position 1
  assert( *((int*) dlist.head->next->data) == 60);

  // check position 2
  assert( *((int*) dlist.head->next->next->data) == 10);

  // check position 3
  assert( *((int*) dlist.head->next->next->next->data) == 50);
  
  // check position 4
  assert( *((int*) dlist.head->next->next->next->next->data) == 40);

  // check position 5
  assert( *((int*) dlist.head->next->next->next->next->next->data) == 20);
  
  // check position 6
  assert( *((int*) dlist.head->next->next->next->next->next->next->data) == 30);

  // check position 7
  assert( *((int*) dlist.head->next->next->next->next->next->next->next->data) == 80);

  // check if tail's next node is NULL
  assert( dlist.head->next->next->next->next->next->next->next->next == NULL);
  assert(ads_dlist_get_tail(&dlist)->next == NULL); // another way to test

  // check head's data
  assert( *((int*) ads_dlist_get_head(&dlist)->data) == 70);

  // check if head's next node data is 10
  assert( *((int*) ads_dlist_get_head(&dlist)->next->data) == 60);  

  // check tail's data
  assert( *((int*) ads_dlist_get_tail(&dlist)->data) == 80);

  // check list size
  assert(ads_dlist_get_size(&dlist) == 8);
}

int main() {

  ads_dlist_add_TEST();

  puts("DOUBLE LINKED LIST TEST: OK");

  return 0;
}