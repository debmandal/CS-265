#include <stddef.h>
#include <stdbool.h>

typedef unsigned int (*hash_function)(const void *data, int hash);

struct bloom_hash {
    hash_function func;
    int hash;
    struct bloom_hash *next;
};

typedef struct bloom_filter {
    struct bloom_hash *func; //list of hash functions
    void *bits;
    int size;
    int n_elements;
} bloom_filter;

/* Creates a new bloom filter with no hash functions and size bytes. */
bloom_filter* create_bloom(int size);

/* Frees a bloom filter. */
void free_bloom(bloom_filter* filter);

/* Adds a hashing function to the bloom filter. */
void add_hash_bloom(bloom_filter* filter, hash_function func, int hash);

/* Adds an item to the bloom filter. */
void add_to_bloom(bloom_filter* filter, void *item);

/* Tests if an item is in the bloom filter.
 * Returns false if the item has definitely not been added before. Returns true
 * if the item was probably added before. */
bool test_bloom(bloom_filter* filter, void *item);

unsigned int djb2(const void* _str, int hash);
void reset_bloom(bloom_filter* bf);
