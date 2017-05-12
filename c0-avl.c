#include "lsmt.h"


/* Create a new AVL tree. */
avl_tree *avl_create() {
	avl_tree *tree = NULL;

	if( ( tree = malloc( sizeof( avl_tree ) ) ) == NULL ) {
		return NULL;
	}

	tree->root = NULL;
	tree->size = 0;

	return tree;
}

/* Initialize a new node. */
avl_node *avl_create_node() {
	avl_node *node = NULL;
	
	if( ( node = malloc( sizeof( avl_node ) ) ) == NULL ) {
		return NULL;
	}

	node->left = NULL;
	node->right = NULL;
	node->key = 0;
	node->value = 0;

	return node;	
}

/* Find the height of an AVL node recursively */
int avl_node_height( avl_node *node ) {
	int height_left = 0;
	int height_right = 0;

	if( node->left ) height_left = avl_node_height( node->left );
	if( node->right ) height_right = avl_node_height( node->right );

	return height_right > height_left ? ++height_right : ++height_left;
}

/* Find the balance of an AVL node */
int avl_balance_factor( avl_node *node ) {
	int bf = 0;

	if( node->left  ) bf += avl_node_height( node->left );
	if( node->right ) bf -= avl_node_height( node->right );

	return bf ;
}

/* Left Left Rotate */
avl_node *avl_rotate_leftleft( avl_node *node ) {
 	avl_node *a = node;
	avl_node *b = a->left;
	
	a->left = b->right;
	b->right = a;

	return( b );
}

/* Left Right Rotate */
avl_node *avl_rotate_leftright( avl_node *node ) {
	avl_node *a = node;
	avl_node *b = a->left;
	avl_node *c = b->right;
	
	a->left = c->right;
	b->right = c->left; 
	c->left = b;
	c->right = a;

	return( c );
}

/* Right Left Rotate */
avl_node *avl_rotate_rightleft( avl_node *node ) {
	avl_node *a = node;
	avl_node *b = a->right;
	avl_node *c = b->left;
	
	a->right = c->left;
	b->left = c->right; 
	c->right = b;
	c->left = a;

	return( c );
}

/* Right Right Rotate */
avl_node *avl_rotate_rightright( avl_node *node ) {
	avl_node *a = node;
	avl_node *b = a->right;
	
	a->right = b->left;
	b->left = a; 

	return( b );
}

/* Balance a given node */
avl_node *avl_balance_node( avl_node *node ) {
	avl_node *newroot = NULL;

	/* Balance our children, if they exist. */
	if( node->left )
		node->left  = avl_balance_node( node->left  );
	if( node->right ) 
		node->right = avl_balance_node( node->right );

	int bf = avl_balance_factor( node );

	if( bf >= 2 ) {
		/* Left Heavy */	

		if( avl_balance_factor( node->left ) <= -1 ) 
			newroot = avl_rotate_leftright( node );
		else 
			newroot = avl_rotate_leftleft( node );

	} else if( bf <= -2 ) {
		/* Right Heavy */

		if( avl_balance_factor( node->right ) >= 1 )
			newroot = avl_rotate_rightleft( node );
		else 
			newroot = avl_rotate_rightright( node );

	} else {
		/* This node is balanced -- no change. */

		newroot = node;
	}

	return( newroot );	
}

/* Balance a given tree */
void avl_balance( avl_tree *tree ) {

	avl_node *newroot = NULL;

	newroot = avl_balance_node( tree->root );

	if( newroot != tree->root )  {
		tree->root = newroot; 
	}
}

/* Insert a new node with the (key,value) pair */
void avl_insert( avl_tree *tree, int key, int value ) {
	avl_node *node = NULL;
	avl_node *next = NULL;
	avl_node *last = NULL;

	/* Well, there must be a first case */ 	
	if( tree->root == NULL ) {
		node = avl_create_node();
		node->value = value;
		node->key = key;
		tree->root = node;

	/* Okay.  We have a root already.  Where do we put this? */
	} else {
		next = tree->root;

		while( next != NULL ) {
			last = next;

			if( key < next->key ) {
				next = next->left;

			} else if( key > next->key ) {
				next = next->right;

			/* Have we already inserted this node? */
			} else if( value == next->value ) {
				/* This shouldn't happen. */	
			}
		}

		node = avl_create_node();
		node->key = key;
		node->value = value;

		if( key < last->key ) last->left = node;
		if( key > last->key ) last->right = node;
		
	}

	tree->size++;
	avl_balance( tree );
}

/* Find the node containing a given key*/
avl_node *avl_find( avl_tree *tree, int key) {
	avl_node *current = tree->root;

	while( current && current->key != key) {
		if( key > current->key)
			current = current->right;
		else
			current = current->left;
	}

	return current;
}

/* Do a depth first traverse of a node. */
void avl_traverse_node_dfs( avl_node *node, int depth ) {
	int i = 0;

	if( node->left ) avl_traverse_node_dfs( node->left, depth + 2 );

//	for( i = 0; i < depth; i++ ) putchar( ' ' );
	printf( "%d:%d:L1 ", node->key, node->value  );

	if( node->right ) avl_traverse_node_dfs( node->right, depth + 2 );
}

/* Do a depth first traverse of a tree. */
void avl_traverse_dfs( avl_tree *tree ) {
	avl_traverse_node_dfs( tree->root, 0 );
}

//flush and deallocate the entries of avl_tree
void flush_tree_to_file(FILE* f, avl_node* node) {
	if(node->left) 
	{
		flush_tree_to_file(f, node->left);
		free(node->left);
	}
	fprintf(f, "%d %d ", node->key, node->value);
	if(node->right)
	{
		flush_tree_to_file(f, node->right);
		free(node->right);
	}
}
