#define bool char
#define false 0
#define true 1

// Default order is 4.
#define DEFAULT_ORDER 4

// TYPES.

typedef struct record {
	int value;
} record;


/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct bt_node {
	void ** pointers;
	int * keys;
	struct bt_node * parent;
	bool is_leaf;
	int num_keys;
	struct bt_node * next; // Used for queue.
} bt_node;


// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
//int order = DEFAULT_ORDER;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
//bt_node * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
//bool verbose_output = false;


// FUNCTION PROTOTYPES.

// find utility.

void enqueue( bt_node * new_node );
bt_node * dequeue( void );
int height( bt_node * root );
int find_range( bt_node * root, int key_start, int key_end, bool verbose,
		int returned_keys[], void * returned_pointers[]); 
bt_node * find_leaf( bt_node * root, int key, bool verbose );
record * find( bt_node * root, int key, bool verbose );
int cut( int length );

// Insertion.

record * make_record(int value);
bt_node * make_node( void );
bt_node * make_leaf( void );
int get_left_index(bt_node * parent, bt_node * left);
bt_node * insert_into_leaf( bt_node * leaf, int key, record * pointer );
bt_node * insert_into_leaf_after_splitting(bt_node * root, bt_node * leaf, int key,
                                        record * pointer);
bt_node * insert_into_node(bt_node * root, bt_node * parent, 
		int left_index, int key, bt_node * right);
bt_node * insert_into_node_after_splitting(bt_node * root, bt_node * parent,
                                        int left_index,
		int key, bt_node * right);
bt_node * insert_into_parent(bt_node * root, bt_node * left, int key, bt_node * right);
bt_node * insert_into_new_root(bt_node * left, int key, bt_node * right);
bt_node * start_new_tree(int key, record * pointer);
bt_node * insert( bt_node * root, int key, int value );




