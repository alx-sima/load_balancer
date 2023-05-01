/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "list.h"
#include "utils.h"

hashtable *ht_create(unsigned int num_buckets, size_t key_size,
					 size_t data_size, unsigned int (*hash_func)(void *))
{
	hashtable *ht = malloc(sizeof(hashtable));
	DIE(!ht, "failed malloc() of hashtable");

	ht->num_buckets = num_buckets;
	ht->buckets = calloc(num_buckets, sizeof(list *));
	DIE(!ht->buckets, "failed malloc() of hashtable.buckets");

	ht->key_size = key_size;
	ht->data_size = data_size;

	ht->hash_func = hash_func;

	return ht;
}

unsigned int ht_compute_hash(hashtable *ht, void *key)
{
	return ht->hash_func(key) % ht->num_buckets;
}

void ht_store_item(hashtable *ht, void *key, void *value)
{
	unsigned int hash = ht_compute_hash(ht, key);
	list_push_item(&ht->buckets[hash], key, value, ht->key_size, ht->data_size);
}

void ht_delete_item(hashtable *ht, void *key)
{
	unsigned int hash = ht_compute_hash(ht, key);
	list *item_node = list_pop_item(&ht->buckets[hash], key, ht->key_size);

	free(item_node->info->key);
	free(item_node->info->data);
	free(item_node->info);
	free(item_node);
}

void *ht_get_item(hashtable *ht, void *key)
{
	unsigned int hash = ht_compute_hash(ht, key);
	return list_get_item(ht->buckets[hash], key, ht->key_size);
}

void *ht_clone_val(hashtable *ht, void *key)
{
	void *ref = ht_get_item(ht, key);
	if (!ref)
		return NULL;

	void *clone = malloc(ht->data_size);
	DIE(!clone, "failed malloc() of data");

	memcpy(clone, ref, ht->data_size);
	return clone;
}

dict_entry *ht_pop_hash_entry(hashtable *ht, unsigned int hash)
{
	list *hash_node = ht->buckets[hash % ht->num_buckets];
	if (!hash_node)
		return NULL;

	dict_entry *entry = hash_node->info;

	ht->buckets[hash] = ht->buckets[hash]->next;
	free(hash_node);

	return entry;
}

void ht_destroy(hashtable *ht)
{
	for (unsigned int i = 0; i < ht->num_buckets; ++i)
		list_destroy(ht->buckets[i]);
	free(ht->buckets);
	free(ht);
}
