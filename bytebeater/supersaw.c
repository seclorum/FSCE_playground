#include <stdio.h>

int main() {
    unsigned int t = 0; // Time step, an unsigned integer
    while (1) {
        putchar(
            (
                (((t * 7) & 255) + ((int)(t * 7.03) & 255) + ((int)(t * 6.97) & 255)) / 3
            )
        );
        t++;
    }
    return 0;
}

