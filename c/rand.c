#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    srand(time(NULL));
    for (int i=0; i<100; i++) {
        printf("%i\n", rand());
    }
}
