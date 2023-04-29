/* Copyright 2023 Sima Alexandru (312CA) */
#include <string.h>

#include "list.h"
#include "utils.h"

void insert_item_list(list **l, void *key, void *value, size_t key_size,
					  size_t data_size)
{
	list *node = malloc(sizeof(list));
	DIE(!node, ""); // TODO

	node->key = malloc(key_size);
	DIE(!node->key, ""); // TODO
	node->data = malloc(data_size);
	DIE(!node->data, ""); // TODO

	memcpy(node->key, key, key_size);
	memcpy(node->data, value, data_size);

	node->next = *l;
	*l = node;
}

list *pop_item_list(list **l, void *key, size_t key_size)
{
	list *prev = NULL;
	list *curr = *l;

	while (curr) {
		if (memcmp(curr->key, key, key_size) == 0) {
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

void *get_item_list(list *l, void *key, size_t key_size)
{
	while (l) {
		if (memcmp(l->key, key, key_size) == 0)
			return l->data;
		l = l->next;
	}

	return NULL;
}

void delete_list(list *l)
{
	while (l) {
		free(l->key);
		free(l->data);
		l = l->next;
	}
}
