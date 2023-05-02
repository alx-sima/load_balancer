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
	list *bucket_iter = ht->buckets[hash];

	/* Nodul se insereaza la inceputul listei */
	if (!bucket_iter || hash < ht_compute_hash(ht, bucket_iter->info->key)) {
		list *new_node =
			list_create_node(key, value, ht->key_size, ht->data_size);
		new_node->next = ht->buckets[hash];
		ht->buckets[hash] = new_node;
		return;
	}

	while (bucket_iter->next) {
		if (hash < ht_compute_hash(ht, bucket_iter->next->info->key))
			break;

		bucket_iter = bucket_iter->next;
	}

	list *new_node = list_create_node(key, value, ht->key_size, ht->data_size);
	new_node->next = bucket_iter->next;
	bucket_iter->next = new_node;
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

/**
 * @brief Scoate primul element gasit, incepand de la un bucket dat.
 *
 * Incepand din bucketul `*bucket`, cauta un element, il scoate din hashtable si
 * il intoarce, retinand la adresa `bucket`, pozitia la care a fost gasit. In
 * acest mod, la cautari ulterioare, nu se va mai itera prin bucketuri care se
 * stie ca sunt goale.
 *
 * @return Un nod de lista care contine prima pereche (cheie, valoare) gasita
 * @return NULL Hashtable-ul nu stocheaza nimic in bucketurile specificate
 */
list *ht_pop_entry(hashtable *ht)
{
	for (int i = 0; i <  ht->num_buckets; ++i) {
		if (ht->buckets[i]) {
			list *found_node = ht->buckets[i];
			ht->buckets[i] = ht->buckets[i]->next;

			found_node->next = NULL;
			return found_node;
		}
	}

	return NULL;
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

list *ht_chain_entries(hashtable *ht, unsigned int max_hash)
{
	list *chain = NULL;
	list *chain_tail = NULL;

	for (unsigned int i = 0; i < ht->num_buckets; ++i) {
		if (!ht->buckets[i])
			continue;
		if (chain_tail &&
			ht_compute_hash(ht, ht->buckets[i]->info->key) < max_hash)
			chain_tail->next = ht->buckets[i];

		while (ht->buckets[i]) {
			if (ht_compute_hash(ht, ht->buckets[i]->info->key) >= max_hash)
				break;

			chain_tail = ht->buckets[i];
			ht->buckets[i] = ht->buckets[i]->next;

			if (!chain)
				chain = chain_tail;
		}
	}

	if (chain_tail)
		chain_tail->next = NULL;
	return chain;
}

void ht_destroy(hashtable *ht)
{
	for (unsigned int i = 0; i < ht->num_buckets; ++i)
		list_destroy(ht->buckets[i]);
	free(ht->buckets);
	free(ht);
}
