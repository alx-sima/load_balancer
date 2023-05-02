/* Copyright 2023 Sima Alexandru (312CA) */
#include <string.h>

#include "list.h"
#include "utils.h"

static void list_delete_node(list *l);

list *list_pop_item(list **l, void *key, size_t key_size)
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

void *list_get_item(list *l, void *key, size_t key_size)
{
	while (l) {
		if (memcmp(l->info->key, key, key_size) == 0)
			return l->info->data;
		l = l->next;
	}

	return NULL;
}

void list_destroy(list *l)
{
	while (l) {
		list *next = l->next;
		list_delete_node(l);
		l = next;
	}
}

list *list_create_node(void *key, void *value, size_t key_size,
					   size_t data_size)
{
	list *node = malloc(sizeof(list));
	DIE(!node, "failed malloc() of list");

	node->info = malloc(sizeof(dict_entry));
	DIE(!node->info, "failed malloc() of list.info");

	node->info->key = malloc(key_size);
	DIE(!node->info->key, "failed malloc() of list.info.key");

	node->info->data = malloc(data_size);
	DIE(!node->info->data, "failed malloc() of list.info.data");

	memcpy(node->info->key, key, key_size);
	memcpy(node->info->data, value, data_size);

	node->next = NULL;
	return node;
}

static void list_delete_node(list *l)
{
	free(l->info->key);
	free(l->info->data);
	free(l->info);
	free(l);
}
