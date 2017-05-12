#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include "c0-avl.h"
#include "bloom.h"

typedef struct {
	avl_tree* c0_tree;	//in-memory avl tree
	unsigned int maxdepth;		//maximum depth of the avl tree
	unsigned int T;			//multiplication factor
	unsigned int* level_sizes;	//sizes of the levels beginning from level 1
	int numlevel;
	FILE** fptrs;		//array of pointers to numlevel files of increasing sizes
	bloom_filter** bloom;	//array of pointers to numlevel bloom filters
	double* false_pos;		//false positive probabilities of different levels
	unsigned int* file_sizes;	//number of entries in each level
	signed int* deleted_nodes;	//list of keys of deleted nodes
	unsigned int maxdeletes;	//maximum number of deleted nodes
	unsigned int curdeletes; // current number of deleted nodes
	int unfiltered; // all levels above unfiltered have fpr 1
} lsmtree;

typedef struct pair {
	int key;
	int value;
} pair;

typedef struct list {
	pair p;
	struct list* next;
} list;
typedef struct subtree {
	pair* subarray;
	int size;
	int maxsize;
} subtree;

void error_msg(int num, ...);


void quick_sort(int* arr, int low, int high);
int partition(int* arr, int low, int high);
void print_stats(lsmtree* l);
void delete_key(lsmtree* l, int key);
list* range(lsmtree* l, int key1, int key2);
int search(lsmtree* l, int key, int* value);
int search_in_file(FILE* f, int start, int end, int key, int* value);
int binsearch(int key, int* arr, int start, int end);
void flush_tree_to_disk(lsmtree* l);
void flush_to_disk(lsmtree* l);
void lsmt_insert(lsmtree* l,  int key, int value);
lsmtree* initialize(unsigned int maxdepth, unsigned int T, int numlevel, double p1, int unfiltered, unsigned int maxdels);
