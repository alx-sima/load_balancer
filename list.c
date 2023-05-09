/* Copyright 2023 Sima Alexandru (312CA) */
#include <string.h>

#include "list.h"
#include "utils.h"

list *list_create_node(void *key, void *value)
{
	list *node = malloc(sizeof(list));
	DIE(!node, "failed malloc() of list");

	node->info.key = key;
	node->info.data = value;

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

void *list_get_item(list *l, void *key, int (*compare_func)(void *, void *))
{
	while (l) {
		if (compare_func(l->info.key, key) == 0)
			return l->info.data;
		l = l->next;
	}

	return NULL;
}

list *list_extract_item(list **l, void *key,
						int (*compare_func)(void *, void *))
{
	list *prev = NULL;
	list *curr = *l;

	while (curr) {
		if (compare_func(curr->info.key, key) == 0) {
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

void list_destroy(list *l, void (*destructor_func)(void *, void *))
{
	while (l) {
		list *next = l->next;

		destructor_func(l->info.key, l->info.data);
		free(l);
		l = next;
	}
}
