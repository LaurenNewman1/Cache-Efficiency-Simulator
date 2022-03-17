/*
 *
 *      Splay tree structure used for priority queues
 */

#include <stdio.h>
#include "splay.h"


/* -- Insert Item to Splay Tree ------------------------------------- */

void init(SplayTree *tr_ptr) {
  *tr_ptr = NULL;
}

/* -- Insert Item to Splay Tree ------------------------------------- */

void insert(SplayNode *e, SplayTree *tr_ptr) {
   register SplayNode *l; /* the rightmost node in the left tree */
   register SplayNode *r; /* the leftmost node in the right tree */
   register SplayNode *nextbl; /* the root of the unsplit part */
   register SplayNode *tmp;

   e->parent = NULL;
   nextbl = *tr_ptr;
   *tr_ptr = e;
   if (nextbl == NULL) {	/* trivial enq */
      e->left = NULL;
      e->right = NULL;
   } else {			/* difficult enq */
      l = e;
      r = e;

      /* e's left and right children will hold the right
         and left splayed trees resulting from splitting on
         e->key; note that the children will be reversed! */

      if (e->key < nextbl->key)
	 goto two;

one:				/* assert nextbl->key <=
				   key */

      do {			/* walk to the right in the
				   left tree */
	 tmp = nextbl->right;
	 if (tmp == NULL) {
	    l->right = nextbl;
	    nextbl->parent = l;
	    r->left = NULL;
	    goto done;		/* job done, entire tree
				   split */
	 }
	 if (e->key < tmp->key) {
	    l->right = nextbl;
	    nextbl->parent = l;
	    l = nextbl;
	    nextbl = tmp;
	    goto two;		/* change sides */
	 }
	 nextbl->right = tmp->left;
	 if (tmp->left != NULL)
	    tmp->left->parent = nextbl;
	 l->right = tmp;
	 tmp->parent = l;
	 tmp->left = nextbl;
	 nextbl->parent = tmp;
	 l = tmp;
	 nextbl = tmp->right;
	 if (nextbl == NULL) {
	    r->left = NULL;
	    goto done;		/* job done, entire tree
				   split */
	 }
      } while (e->key >= nextbl->key); /* change sides */

two:				/* assert nextbl->key > key */

      do {			/* walk to the left in the
				   right tree */
	 tmp = nextbl->left;
	 if (tmp == NULL) {
	    r->left = nextbl;
	    nextbl->parent = r;
	    l->right = NULL;
	    goto done;		/* job done, entire tree
				   split */
	 }
	 if (e->key >= tmp->key) {
	    r->left = nextbl;
	    nextbl->parent = r;
	    r = nextbl;
	    nextbl = tmp;
	    goto one;		/* change sides */
	 }
	 nextbl->left = tmp->right;
	 if (tmp->right != NULL)
	    tmp->right->parent = nextbl;
	 r->left = tmp;
	 tmp->parent = r;
	 tmp->right = nextbl;
	 nextbl->parent = tmp;
	 r = tmp;
	 nextbl = tmp->left;
	 if (nextbl == NULL) {
	    l->right = NULL;
	    goto done;		/* job done, entire tree
				   split */
	 }
      } while (e->key < nextbl->key); /* change sides */

      goto one;

done:				/* split is done, branches
				   of e need reversal */

      tmp = e->left;
      e->left = e->right;
      e->right = tmp;
   }
}

/* -- Delete Item from Splay Tree ----------------------------------- */

void rm(SplayNode **e, SplayTree *tr_ptr) {
   SplayNode *n;
   SplayNode *nextbl; /* the next thing to deal with */
   SplayNode *left;	/* the left child of next */
   register SplayNode *farleft; /* the left child of left */
   register SplayNode *farfarleft; /* the left child of farleft */

   n = *tr_ptr;
   if (n == NULL) { 
      *e = NULL;
   } else {
      /* if left is NULL this is the deq block but also we
         have to reset the root.  JJW */
      nextbl = n;
      left = nextbl->left;
      if (left == NULL) {
	 *e = nextbl;
	 n = nextbl->right;
	 if (n != NULL) {
	    n->parent = NULL;
	    *tr_ptr = n;
	 } else
	    *tr_ptr = NULL;
      } else
	 for (;;) {
	    /* next is not it, left is not NULL, might be
	       it */
	    farleft = left->left;
	    if (farleft == NULL) {
	       *e = left;
	       nextbl->left = left->right;
	       if (left->right != NULL)
		  left->right->parent = nextbl;
	       break;
	    }
	    /* next, left are not it, farleft is not NULL,
	       might be it */
	    farfarleft = farleft->left;
	    if (farfarleft == NULL) {
	       *e = farleft;
	       left->left = farleft->right;
	       if (farleft->right != NULL)
		  farleft->right->parent = left;
	       break;
	    }
	    /* next, left, farleft are not it, rotate */
	    nextbl->left = farleft;
	    farleft->parent = nextbl;
	    left->left = farleft->right;
	    if (farleft->right != NULL)
	       farleft->right->parent = left;
	    farleft->right = left;
	    left->parent = farleft;
	    nextbl = farleft;
	    left = farfarleft;
	 }
   }
}

