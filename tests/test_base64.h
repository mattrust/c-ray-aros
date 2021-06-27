//
//  test_base64.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 18/03/2021.
//  Copyright © 2021 Valtteri Koskivuori. All rights reserved.
//

#include "../src/utils/base64.h"
#include "../src/utils/string.h"
#include "../src/utils/fileio.h"

bool base64_basic(void) {
	char *original = "This is the original string right here.";
	char *encoded = b64encode(original, strlen(original));
	char *decoded = b64decode(encoded, strlen(encoded), NULL);
	test_assert(stringEquals(original, decoded));
	
	free(encoded);
	free(decoded);

	return true;
}

bool base64_padding_2(void) {
	char *original = "a";
	char *encoded = b64encode(original, strlen(original));
	test_assert(strlen(encoded) == 4);
	size_t decoded_length = 0;
	char *decoded = b64decode(encoded, strlen(encoded), &decoded_length);
	test_assert(strlen(decoded) == strlen(original));
	test_assert(stringEquals(original, decoded));
	
	free(encoded);
	free(decoded);
	
	return true;
}

bool base64_padding_1(void) {
	char *original = "aa";
	char *encoded = b64encode(original, strlen(original));
	test_assert(strlen(encoded) == 4);
	size_t decoded_length = 0;
	char *decoded = b64decode(encoded, strlen(encoded), &decoded_length);
	test_assert(strlen(decoded) == strlen(original));
	test_assert(stringEquals(original, decoded));
	
	free(encoded);
	free(decoded);
	
	return true;
}

bool base64_padding_0(void) {
	char *original = "aaa";
	char *encoded = b64encode(original, strlen(original));
	test_assert(strlen(encoded) == 4);
	size_t decoded_length = 0;
	char *decoded = b64decode(encoded, strlen(encoded), &decoded_length);
	test_assert(strlen(decoded) == strlen(original));
	test_assert(stringEquals(original, decoded));
	
	free(encoded);
	free(decoded);
	
	return true;
}
