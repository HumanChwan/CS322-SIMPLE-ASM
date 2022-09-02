#include <stdio.h>
#include <stdlib.h>

#define MAX 100

int main() {
    FILE *f;
    /* char *line = (char*)calloc(sizeof(char), 100); */
    char line[MAX];
    f = fopen("tests/examples/test1.asm", "r");
    if (f == NULL) {
        printf("Error reading");
        return -1;
    }
    while (fgets(line, 100, f) != NULL) {
        printf("%s", line);
    }
    return 0;
}