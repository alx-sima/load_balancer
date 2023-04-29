/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

hashtable *new_ht(unsigned int num_buckets, size_t data_size,
				  int (*hash_func)(void *))
{
	hashtable *ht = malloc(sizeof(hashtable));
	if (!ht)
		return NULL;

	ht->num_buckets = num_buckets;
	ht->data_size = data_size;
	ht->hash_func = hash_func;
	ht->buckets = calloc(num_buckets, sizeof(list *));
	if (!ht->buckets) {
		free(ht);
		return NULL;
	}
	return ht;
}

int insert_value_list(list **l, void *value, size_t data_size)
{
	list *node = malloc(sizeof(list));
	if (!node)
		return 0;

	node->data = malloc(data_size);
	if (!node->data) {
		free(node);
		return 0;
	}
	memcpy(node->data, value, data_size);

	node->next = *l;
	*l = node;
	return 1;
}

int insert_value_ht(hashtable *ht, void *value)
{
	unsigned int hash = ht->hash_func(value) % ht->num_buckets;
	return insert_value_list(&ht->buckets[hash], value, ht->data_size);
}

void delete_list(list *l)
{
	while (l) {
		free(l->data);
		l = l->next;
	}
}

void delete_ht(hashtable *ht)
{
	for (unsigned int i = 0; i < ht->num_buckets; ++i)
		delete_list(ht->buckets[i]);
	free(ht->buckets);
	free(ht);
}
