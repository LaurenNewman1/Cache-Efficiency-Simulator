#ifndef _SPLAY_H_
#define _SPLAY_H_

/*
 *      Splay tree node
 */
typedef struct SplayNode_Struct {
  struct SplayNode_Struct *next;          /* link nodes into list */
  struct SplayNode_Struct *left;          /* Splay tree internals */
  struct SplayNode_Struct *right;         /* Splay tree internals */
  struct SplayNode_Struct *parent;        /* Splay tree internals */
  double              key;                /* key value for sorting */
} SplayNode;

typedef struct SplayNode_Struct* SplayTree;

/* initialize the tree */
void init(SplayTree *tr_ptr);

/* -- Insert Item to Splay Tree ------------------------------------- */
void insert(SplayNode *e, SplayTree *tr_ptr);

/* -- Delete (Remove) Item from Splay Tree -------------------------------- */
void rm(SplayNode **e, SplayTree *tr_ptr);

#endif
