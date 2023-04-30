/* Copyright 2023 Sima Alexandru (312CA) */
#include <string.h>

#include "list.h"
#include "utils.h"

list *create_node(void *key, void *value, size_t key_size, size_t data_size)
{
	list *node = malloc(sizeof(list));
	DIE(!node, ""); // TODO

	node->info = malloc(sizeof(dict_entry));
	DIE(!node->info, ""); // TODO

	node->info->key = malloc(key_size);
	DIE(!node->info->key, ""); // TODO

	node->info->data = malloc(data_size);
	DIE(!node->info->data, ""); // TODO

	memcpy(node->info->key, key, key_size);
	memcpy(node->info->data, value, data_size);

	node->next = NULL;
	return node;
}

void insert_item_list(list **l, void *key, void *value, size_t key_size,
					  size_t data_size)
{
	list *node = create_node(key, value, key_size, data_size);
	node->next = *l;
	*l = node;
}

list *pop_item_list(list **l, void *key, size_t key_size)
{
	list *prev = NULL;
	list *curr = *l;

	while (curr) {
		if (memcmp(curr->info->key, key, key_size) == 0) {
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
		if (memcmp(l->info->key, key, key_size) == 0)
			return l->info->data;
		l = l->next;
	}

	return NULL;
}

void delete_node(list *l)
{
	free(l->info->key);
	free(l->info->data);
	free(l->info);
	free(l);
}

void delete_list(list *l)
{
	while (l) {
		list *next = l->next;
		delete_node(l);
		l = next;
	}
}
