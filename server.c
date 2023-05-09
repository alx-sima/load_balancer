/* Copyright 2023 Sima Alexandru (312CA) */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "server.h"
#include "utils.h"

#define BUCKET_NO 421

struct server_memory {
	/** hashtable care contine
	 *obiectele stocate pe server */
	hashtable *database;
};

static int compare_server_entries(void *a, void *b)
{
	return strcmp((char *)a, (char *)b);
}

static void free_server_entry(void *key, void *data)
{
	free(key);
	free(data);
}

static char *copy_string(char *s)
{
	char *copy = malloc(strlen(s) + 1);
	DIE(!copy, "failed malloc() of copy");

	strcpy(copy, s);
	return copy;
}

server_memory *init_server_memory()
{
	struct server_memory *server = malloc(sizeof(struct server_memory));
	DIE(!server, "failed malloc() of server_memory");

	server->database = ht_create(BUCKET_NO, hash_function_key,
								 compare_server_entries, free_server_entry);
	DIE(!server->database, "failed malloc() of server_memory.database");
	return server;
}

void server_store(server_memory *server, char *key, char *value)
{
	char *key_copy = copy_string(key);
	DIE(!key_copy, "failed strdup() of key");

	char *value_copy = copy_string(value);
	DIE(!value_copy, "failed strdup() of value");

	ht_store_item(server->database, key_copy, value_copy);
}

char *server_retrieve(server_memory *server, char *key)
{
	return ht_retrieve_item(server->database, key);
}

void server_remove(server_memory *server, char *key)
{
	ht_remove_item(server->database, key);
}

void free_server_memory(server_memory *server)
{
	ht_destroy(server->database);
	free(server);
}

void transfer_items(server_memory *dest, server_memory *src,
					unsigned int min_hash, unsigned int max_hash)
{
	ht_transfer_items(dest->database, src->database, min_hash, max_hash);
}
