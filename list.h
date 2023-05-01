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

void insert_item_list(list **l, void *key, void *value, size_t key_size,
					  size_t data_size);
list *pop_item_list(list **l, void *key, size_t key_size);

void *get_item_list(list *l, void *key, size_t key_size);

void delete_list(list *l);
#endif /* LIST_H_ */
