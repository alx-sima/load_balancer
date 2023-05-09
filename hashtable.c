/* Copyright 2023 Sima Alexandru (312CA) */
#include <stddef.h>
#include <stdlib.h>

#include "hashtable.h"
#include "list.h"
#include "utils.h"

/**
 * Calculeaza hashul cheii, folosind functia corespunzatoare
 * hashtable-ului.
 */
static unsigned int ht_compute_hash(hashtable *ht, void *key)
{
	return ht->hash_func(key) % ht->num_buckets;
}

hashtable *ht_create(unsigned int num_buckets,
					 unsigned int (*hash_func)(void *),
					 int (*compare_func)(void *, void *),
					 void (*destructor_func)(void *, void *))
{
	hashtable *ht = malloc(sizeof(hashtable));
	DIE(!ht, "failed malloc() of hashtable");

	ht->num_buckets = num_buckets;
	ht->buckets = calloc(num_buckets, sizeof(list *));
	DIE(!ht->buckets, "failed malloc() of hashtable.buckets");

	ht->hash_func = hash_func;
	ht->compare_func = compare_func;
	ht->destructor_func = destructor_func;

	return ht;
}

void ht_store_item(hashtable *ht, void *key, void *value)
{
	unsigned int hash = ht_compute_hash(ht, key);
	list *new_node = list_create_node(key, value);
	list_push(&ht->buckets[hash], new_node);
}

void *ht_retrieve_item(hashtable *ht, void *key)
{
	unsigned int hash = ht_compute_hash(ht, key);
	return list_get_item(ht->buckets[hash], key, ht->compare_func);
}

void ht_remove_item(hashtable *ht, void *key)
{
	unsigned int hash = ht_compute_hash(ht, key);
	list *item_node =
		list_extract_item(&ht->buckets[hash], key, ht->compare_func);

	ht->destructor_func(item_node->info.key, item_node->info.data);
	free(item_node);
}

void ht_transfer_items(hashtable *dest, hashtable *src, unsigned int min_hash,
					   unsigned int max_hash)
{
	for (size_t i = 0; i < src->num_buckets; ++i) {
		if (!src->buckets[i])
			continue;

		list *accepted = NULL;
		list *rejected = NULL;
		list_split(src->buckets[i], &accepted, &rejected, min_hash, max_hash);

		src->buckets[i] = rejected;
		while (accepted) {
			dict_entry pair = accepted->info;
			ht_store_item(dest, pair.key, pair.data);

			list *oldptr = accepted;
			accepted = accepted->next;
			free(oldptr);
		}
	}
}

void ht_destroy(hashtable *ht)
{
	for (size_t i = 0; i < ht->num_buckets; ++i)
		list_destroy(ht->buckets[i], ht->destructor_func);
	free(ht->buckets);
	free(ht);
}
