/* Copyright 2023 Sima Alexandru (312CA) */
#include <string.h>

#include "list.h"
#include "utils.h"

list *list_create_node(void *key, void *value, size_t key_size,
					   size_t data_size)
{
	list *node = malloc(sizeof(list));
	DIE(!node, "failed malloc() of list");

	node->info.key = malloc(key_size);
	DIE(!node->info.key, "failed malloc() of list.info.key");

	node->info.data = malloc(data_size);
	DIE(!node->info.data, "failed malloc() of list.info.data");

	memcpy(node->info.key, key, key_size);
	memcpy(node->info.data, value, data_size);

	node->next = NULL;
	return node;
}

void list_push(list **l, list *node)
{
	if (!*l) {
		node->next = NULL;
		*l = node;
		return;
	}

	node->next = *l;
	*l = node;
}

void *list_get_item(list *l, void *key, size_t key_size)
{
	while (l) {
		if (memcmp(l->info.key, key, key_size) == 0)
			return l->info.data;
		l = l->next;
	}

	return NULL;
}

list *list_extract_item(list **l, void *key, size_t key_size)
{
	list *prev = NULL;
	list *curr = *l;

	while (curr) {
		if (memcmp(curr->info.key, key, key_size) == 0) {
			if (prev)
				prev->next = curr->next;
			else
				*l = curr->next;
			return curr;
		}

		prev = curr;
		curr = curr->next;
	}

	return NULL;
}

void list_split(list *src, list **accepted, list **rejected,
				unsigned int min_hash, unsigned int max_hash)
{
	while (src) {
		list *curr = src;
		src = src->next;

		unsigned int hash = hash_function_key(curr->info.key);
		if (min_hash <= hash && hash < max_hash)
			list_push(accepted, curr);
		else
			list_push(rejected, curr);
	}
}

void list_destroy(list *l)
{
	while (l) {
		list *next = l->next;
		fprintf(stderr, "destroyed %s\n", l->info.key);
		free(l->info.key);
		free(l->info.data);
		free(l);
		l = next;
	}
}
