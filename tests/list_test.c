#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../src/list/list.h"


static inline void ads_list_add_TEST(void) {
  ads_list_t list = {0};
  ads_list_init(&list, NULL);

  int t[] = {10, 20, 30, 40, 50, 60};
  ads_list_node_t* node = NULL;

  // check list size
  assert(ads_list_get_size(&list) == 0);

  // add at head
  assert(!ads_list_add_front(&list, &t[0]));
  
  // check if head and tail points to the same node
  assert(ads_list_get_head(&list) == ads_list_get_tail(&list));

  // add at tail
  assert(!ads_list_add_back(&list, &t[1]));
  
  // check if head and tail points to different nodes
  assert(ads_list_get_head(&list) != ads_list_get_tail(&list));

  // get tail
  node = ads_list_get_tail(&list);

  // add at end using `node`
  assert(!ads_list_add_next(&list, node, &t[2]));

  // get head
  node = ads_list_get_head(&list);
  
  // add after head using `node`
  assert(!ads_list_add_next(&list, node, &t[3]));

  // add at head
  assert(!ads_list_add_front(&list, &t[4]));

  // add at tail
  assert(!ads_list_add_back(&list, &t[5]));

  /* ======= at this point, the list must be [50, 10, 40, 20, 30, 60] ======= */

  // check position 0 (head)
  assert( *((int*) list.head->data) == 50);

  // check position 1
  assert( *((int*) list.head->next->data) == 10);

  // check position 2
  assert( *((int*) list.head->next->next->data) == 40);

  // check position 3
  assert( *((int*) list.head->next->next->next->data) == 20);
  
  // check position 4
  assert( *((int*) list.head->next->next->next->next->data) == 30);

  // check position 5 (tail)
  assert( *((int*) list.head->next->next->next->next->next->data) == 60);

  // check if tail's next node is NULL
  assert(list.head->next->next->next->next->next->next == NULL);
  assert(ads_list_get_tail(&list)->next == NULL); // another way to test

  // check head's data
  assert( *((int*) ads_list_get_head(&list)->data) == 50);

  // check if head's next node data is 10
  assert( *((int*) ads_list_get_head(&list)->next->data) == 10);  

  // check tail's data
  assert( *((int*) ads_list_get_tail(&list)->data) == 60);

  // check list size
  assert(ads_list_get_size(&list) == 6);

  ads_list_destroy(&list);
}

static inline void ads_list_remove_TEST(void) {
  ads_list_t list = {0};
  ads_list_init(&list, NULL);

  int t[] = {10, 20, 30, 40, 50, 60};
  int* ret[6] = {0};
  ads_list_node_t* node = NULL;

  ads_list_add_front(&list, &t[0]);
  ads_list_add_back(&list, &t[1]);
  node = ads_list_get_tail(&list);
  ads_list_add_next(&list, node, &t[2]);
  node = ads_list_get_head(&list);
  ads_list_add_next(&list, node, &t[3]);
  ads_list_add_front(&list, &t[4]);
  ads_list_add_back(&list, &t[5]);
  
  /* ======= at this point, the list must be [50, 10, 40, 20, 30, 60] ======= */

  // get node after head
  node = ads_list_get_next(ads_list_get_head(&list));

  // remove and get the next element after node (40)
  assert(!ads_list_remove_next(&list, node, (void*) &ret[0]));

  // remove from head
  assert(!ads_list_remove_front(&list, (void*) &ret[1]));

  // remove from tail
  assert(!ads_list_remove_back(&list, (void*) &ret[2]));

  /* ======= 
    at this point, the list must be [10, 20, 30]
    and ret[0] = 40, ret[1] = 50, ret[2] = 60
  ======= */

  // check if ret[0] is equal 40
  assert(*ret[0] == 40);

  // check if ret[1] is equal 50
  assert(*ret[1] == 50);

  // check if ret[2] is equal 60
  assert(*ret[2] == 60);

  // check if the list is not empty
  assert(!ads_list_is_empty(&list));
  
  // check size
  assert(ads_list_get_size(&list) == 3);

  // remove from head using NULL as second parameter
  assert(!ads_list_remove_next(&list, NULL, (void*) &ret[3]));

  // check if ret[3] is 10
  assert(*ret[3] == 10);

  // remove from tail
  assert(!ads_list_remove_back(&list, (void*) &ret[4]));

  // check if ret[4] is 10
  assert(*ret[4] == 30);

  // check size
  assert(ads_list_get_size(&list) == 1);

  // check if head and tail points to the same node
  assert(ads_list_get_head(&list) == ads_list_get_tail(&list));

  // try to remove tail's next node; must return -1
  assert(ads_list_remove_next(&list, ads_list_get_tail(&list), (void*) &ret[5]) == -1);

  // check if ret[5] is NULL
  assert(ret[5] == NULL);

  // remove from head, but doesn't save the data
  assert(!ads_list_remove_next(&list, NULL, NULL));

  // check if ret[5] is still NULL
  assert(ret[5] == NULL);

  // check if the list is empty
  assert(ads_list_is_empty(&list));

  ads_list_destroy(&list);
}

static inline void ads_list_get_TEST(void) {
  ads_list_t list = {0};
  ads_list_init(&list, NULL);

  int t[] = {10, 20, 30, 40, 50, 60};
  ads_list_node_t* node = NULL;

  ads_list_add_front(&list, &t[0]);
  ads_list_add_back(&list, &t[1]);
  node = ads_list_get_tail(&list);
  ads_list_add_next(&list, node, &t[2]);
  node = ads_list_get_head(&list);
  ads_list_add_next(&list, node, &t[3]);
  ads_list_add_front(&list, &t[4]);
  ads_list_add_back(&list, &t[5]);

  /* ======= at this point, the list must be [50, 10, 40, 20, 30, 60] ======= */

  // try to get a node out of bounds, must return NULL
  assert(ads_list_get_at(&list, 999999) == NULL);
  assert(ads_list_get_at(&list, -1) == NULL);

  // get the fourth node
  node = ads_list_get_at(&list, 3);

  // check if node is the same as list.head->next->next->next 
  assert(node == list.head->next->next->next);

  // check node data
  assert( *((int*) node->data) == 20);

  // get node's next node
  node = ads_list_get_next(node);

  // check if node is the same as list.head->next->next->next->next 
  assert(node == list.head->next->next->next->next);

  // check node data
  assert( *((int*) node->data) == 30);

  // get first node
  node = ads_list_get_at(&list, 0);

  // check if node is the same as list.head
  assert(node == list.head);

  // get the last node
  node = ads_list_get_at(&list, ads_list_get_size(&list) - 1);

  // check if node is the same as list.tail
  assert(node == list.tail);

  // get the fifth node
  node = ads_list_get_at(&list, ads_list_get_size(&list) - 2);

  // check if node->next is the same as list.tail
  assert(node->next == list.tail);

  ads_list_destroy(&list);
}

int main() {

  ads_list_add_TEST();
  ads_list_remove_TEST();
  ads_list_get_TEST();

  puts("LIST TEST: OK");

  return 0;
}