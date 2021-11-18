#ifndef __NODE_H
#define __NODE_H
#include "token.h"

typedef struct node {
  int          type;
  struct node *left;
  struct node *right;
} NODE;

NODE* make_leaf(TOKEN*);
NODE* make_node(int, NODE*, NODE*);

#endif
