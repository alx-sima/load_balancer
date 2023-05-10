/* Copyright 2023 Sima Alexandru (312CA) */
#include "hashring.h"
#include "utils.h"

int compare_servers(const void *a, const void *b)
{
	const hashring_entry *a_cast = a;
	const hashring_entry *b_cast = b;

	if (a_cast->hash != b_cast->hash)
		return a_cast->hash < b_cast->hash ? -1 : 1;
	return a_cast->label < b_cast->label ? -1 : 1;
}

hashring_entry *find_server(hashring_entry *hashring, size_t hashring_size,
							unsigned int target_hash, bool search_containing)
{
	size_t left = 0;
	size_t right = hashring_size - 1;

	while (left <= right) {
		size_t index = (left + right) / 2;
		unsigned int hash = hash_function_servers(&hashring[index].label);

		if (hash == target_hash)
			return &hashring[index];

		if (hash < target_hash) {
			left = index + 1;
			continue;
		}

		right = index - 1;
		if (search_containing) {
			if (index == 0)
				return &hashring[0];

			unsigned int previous_hash =
				hash_function_servers(&hashring[index - 1].label);

			/* Hash-ul este cuprins intre cel al serverului
			 * curent si hashul serverului anterior, deci
			 * elementul apartine serverului curent. */
			if (previous_hash < target_hash)
				return &hashring[index];
		}
	}

	/* Daca nu a fost gasit un server care sa contina hashul
	 * (valabil pentru `search_containing == 1`),
	 * elementul revine primului server. */
	return search_containing ? &hashring[0] : NULL;
}
