/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "list.h"
#include "utils.h"

hashtable *create_ht(unsigned int num_buckets, size_t key_size,
					 size_t data_size, int (*hash_func)(void *))
{
	hashtable *ht = malloc(sizeof(hashtable));
	DIE(!ht, ""); // TODO

	ht->num_buckets = num_buckets;
	ht->buckets = calloc(num_buckets, sizeof(list *));
	DIE(!ht->buckets, ""); // TODO

	ht->key_size = key_size;
	ht->data_size = data_size;

	ht->hash_func = hash_func;

	return ht;
}

void insert_item_ht(hashtable *ht, void *key, void *value)
{
	unsigned int hash = ht->hash_func(key) % ht->num_buckets;
	insert_item_list(&ht->buckets[hash], key, value, ht->key_size,
					 ht->data_size);
}

void remove_item_ht(hashtable *ht, void *key)
{
	unsigned int hash = ht->hash_func(key) % ht->num_buckets;
	list *item_node = pop_item_list(&ht->buckets[hash], key, ht->key_size);

	free(item_node->key);
	free(item_node->data);
	free(item_node);
}

void *get_item_ht(hashtable *ht, void *key)
{
	unsigned int hash = ht->hash_func(key) % ht->num_buckets;
	return get_item_list(ht->buckets[hash], key, ht->key_size);
}

void delete_ht(hashtable *ht)
{
	for (unsigned int i = 0; i < ht->num_buckets; ++i)
		delete_list(ht->buckets[i]);
	free(ht->buckets);
	free(ht);
}
