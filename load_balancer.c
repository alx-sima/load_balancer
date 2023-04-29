/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

struct load_balancer {
	/* TODO 0 */
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer *init_load_balancer()
{
	/* TODO 1 */
	return NULL;
}

void loader_add_server(load_balancer *main, int server_id)
{
	/* TODO 2 */
	(void)main;
	(void)server_id;
}

void loader_remove_server(load_balancer *main, int server_id)
{
	/* TODO 3 */
	(void)main;
	(void)server_id;
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	/* TODO 4 */
	(void)main;
	(void)key;
	(void)value;
	(void)server_id;
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	/* TODO 5 */
	(void)main;
	(void)key;
	(void)server_id;
	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	/* TODO 6 */
	(void)main;
}
