#include <stdio.h>
#include <stdlib.h>
#include "bloom.h"


bloom_filter* create_bloom(unsigned int size) {
  bloom_filter* res = calloc(1, sizeof(struct bloom_filter));
  res->n_elements = 0;
  res->size = size;
  res->bits = calloc(size, 8);
  if(res->bits == NULL)
	  error_msg(1, "cannot allocate bloom_filter\n");
  return res;
}

void reset_bloom(bloom_filter* bf)
{
	free(bf->bits);
	bf->bits = calloc(bf->size,8);
	bf->n_elements = 0;
}

void free_bloom(bloom_filter* filter) {
  if (filter) {
    while (filter->func) {
      struct bloom_hash *h;
      filter->func = h->next;
      free(h);
    }
    free(filter->bits);
    free(filter);
  }
}


void add_hash_bloom(bloom_filter* filter, hash_function func, int hash) {
  struct bloom_hash *h = calloc(1, sizeof(struct bloom_hash));
  h->func = func;
  h->hash = hash;
  struct bloom_hash *last = filter->func;
  while (last && last->next) {
    last = last->next;
  }
  if (last) {
    last->next = h;
  } else {
    filter->func = h;
  }
}


void add_to_bloom(bloom_filter* filter, void* item) {
  filter->n_elements++;
  struct bloom_hash *h = filter->func;
  uint8_t *bits = filter->bits;
  while (h) {
    unsigned int hash = h->func(item, h->hash);
    hash %= filter->size * 8;
    bits[hash / 8] |= 1 << hash % 8;
    h = h->next;
  }
}


bool test_bloom(bloom_filter* filter, void *item) {
  struct bloom_hash *h = filter->func;
  uint8_t *bits = filter->bits;
  while (h) {
    unsigned int hash = h->func(item, h->hash);
    hash %= filter->size * 8;
    if (!(bits[hash / 8] & 1 << hash % 8)) {
      return false;
    }
    h = h->next;
  }
  return true;
}

unsigned int djb2(const void *_str, int hash) {
  const char *str = _str;
  char c;
  for (int i = 0; i < sizeof(signed int); i++) {
    c = *str++;
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

/*
unsigned int jenkins(const void *_str, int hash_0) {
  const char *key = _str;
  unsigned int hash;
  for (int i = 0; i < sizeof(signed int); i++) {
    hash += *key;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    key++;
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}
*/
