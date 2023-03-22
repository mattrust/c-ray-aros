//
//  nodebase.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 07/12/2020.
//  Copyright © 2020-2022 Valtteri Koskivuori. All rights reserved.
//

#pragma once

#include "../utils/logging.h"
#include <stdbool.h>

// Magic for comparing two nodes

struct node_storage;

struct nodeBase {
	bool (*compare)(const void *, const void *);
	char *(*dump)(const void *);
};

bool compareNodes(const void *A, const void *B);

#define HASH_CONS(hashtable, hash, T, ...) \
	{ \
		const T candidate = __VA_ARGS__; \
        struct nodeBase *c = (struct nodeBase *)&candidate; \
		char *dump = c->dump ? c->dump(c) : NULL; \
		const uint32_t h = hash(&candidate); \
		const T *existing = findInHashtable(hashtable, &candidate, h); \
		if (existing) {\
			logr(debug, "Reusing existing %s%s %s%s%s\n", KGRN, &#T[7], KBLU, dump ? dump : "", KNRM);\
			return (void *)existing; \
		} \
		logr(debug, "Inserting new %s%s %s%s%s\n", KRED, &#T[7], KBLU, dump ? dump : "", KNRM); \
		if (dump) free(dump); \
		insertInHashtable(hashtable, &candidate, sizeof(T), h); \
		return findInHashtable(hashtable, &candidate, h); \
	}
