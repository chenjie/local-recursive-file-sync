#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 8

char *hash(FILE *f) {
    char *hash_val = malloc(BLOCK_SIZE);
    // Initialize all block_size bytes.
    for(int i = 0; i < BLOCK_SIZE; i++) {
        hash_val[i] = '\0';
    }

    char input[2];
    int index = 0;
    // Read the input one character at a time.
    while(fread(input, 1, 1, f) != 0) {
        hash_val[index] = hash_val[index] ^ input[0];

        // Make the index a cycle of the BLOCK_SIZE.
        if (index < BLOCK_SIZE - 1) {
            index++;
        } else if (index == (BLOCK_SIZE - 1)) {
            index = 0;
        }
    }
    return hash_val;
}
