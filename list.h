/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef LIST_H_
#define LIST_H_
#include <stddef.h>

typedef struct {
	void *key;
	void *data;
} dict_entry;

typedef struct list {
	dict_entry *info;
	struct list *next;
} list;

list *list_create_node(void *key, void *value, size_t key_size,
							  size_t data_size);
void list_push_item(list **l, void *key, void *value, size_t key_size,
					size_t data_size);

list *list_pop_item(list **l, void *key, size_t key_size);

void *list_get_item(list *l, void *key, size_t key_size);

void list_destroy(list *l);
#endif /* LIST_H_ */
