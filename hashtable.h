/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_
#include <stddef.h>

typedef struct list {
	void *data;
	struct list *next;
} list;

typedef struct hashtable {
	unsigned int num_buckets;
	size_t data_size;
	list **buckets;
	int (*hash_func)(void *);
} hashtable;

hashtable *new_ht(unsigned int num_buckets, size_t data_size, int (*hash_func)(void *));

int insert_value_ht(hashtable *ht, void *value);

void delete_ht(hashtable *ht);

#endif /* HASHTABLE_H_ */
