#include <stdio.h>
#include <string.h>
// #include <strings.h>
#include "lightcjson.h"

int test_removespacing();

int main() {
    printf("\nA sample C program\n\n");
    test_removespacing();
    return 0;
}

int test_removespacing() {
    char in[] = "{JSON FILE HERE}\0";
    char out[] = "{JSONFILEHERE}\0";
    char *ptr;
    printf("input: %s\n", in);
    ptr = jsonRemoveSpacing(in, in);
    printf("output (old): %s\n", in);
    printf("output (ptr): %s\n", ptr);
    int i = strcmp(in, out);
    printf("compare: %d\n", i);
    return 0;
}