typedef struct avl_node {
	struct avl_node *left;
	struct avl_node *right;
	int key;
	int value;
} avl_node;

typedef struct avl_tree {
	avl_node *root;
	int size;
} avl_tree;

avl_tree *avl_create(); 	
avl_node *avl_create_node();
int avl_node_height( avl_node *node ); 
int avl_balance_factor( avl_node *node ); 
//different rotations
avl_node *avl_rotate_leftleft( avl_node *node ); 
avl_node *avl_rotate_leftright( avl_node *node ); 
avl_node *avl_rotate_rightleft( avl_node *node ); 
avl_node *avl_rotate_rightright( avl_node *node ); 

avl_node *avl_balance_node( avl_node *node ); 
void avl_balance( avl_tree *tree ); 
void avl_insert( avl_tree *tree, int key, int value ); 
avl_node *avl_find( avl_tree *tree, int key); 
void avl_traverse_node_dfs( avl_node *node, int depth ); 
void avl_traverse_dfs( avl_tree *tree ); 
void flush_tree_to_file(FILE* f, avl_node* node); 
