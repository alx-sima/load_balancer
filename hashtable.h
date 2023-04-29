/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_
#include <stddef.h>

#include "list.h"

typedef struct hashtable {
	unsigned int num_buckets;
	list **buckets;

	size_t data_size;
	size_t key_size;

	int (*hash_func)(void *);
} hashtable;

hashtable *create_ht(unsigned int num_buckets, size_t data_size, size_t key_size, int (*hash_func)(void *));

void insert_item_ht(hashtable *ht, void *key, void *value);
void remove_item_ht(hashtable *ht, void *key);
void *get_item_ht(hashtable *ht, void *key);

void delete_ht(hashtable *ht);

#endif /* HASHTABLE_H_ */
