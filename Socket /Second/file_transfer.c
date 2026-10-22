#include <stdio.h>

int main() {
    FILE *src, *dest;
    char ch;

    // open source file in read mode
    src = fopen("source.txt", "r");
    if (src == NULL) {
        printf("Source file not found!\n");
        return 1;
    }

    // open destination file in write mode
    dest = fopen("dest.txt", "w");

    // copy character by character
    while ((ch = fgetc(src)) != EOF) {
        fputc(ch, dest);
    }

    printf("File copied successfully!\n");

    fclose(src);
    fclose(dest);
    return 0;
}