/* Copyright 2023 Sima Alexandru (312CA) */
#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)                                       \
	do {                                                                       \
		if (assertion) {                                                       \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
			perror(call_description);                                          \
			exit(errno);                                                       \
		}                                                                      \
	} while (0)

/**
 * @brief Calculeaza hashul asociat unui server.
 *
 * @param a labelul serverului
 *
 * @return hashul calculat
 */
unsigned int hash_function_servers(void *a);

/**
 * @brief Calculeaza hashul unei chei.
 *
 * @param a stringul reprezentand cheia
 *
 * @return hashul calculat
 */
unsigned int hash_function_key(void *a);

#endif /* UTILS_H_ */
