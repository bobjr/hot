#include <stdio.h>

void myop() {
    printf("hi\n");
}

#ifdef TEST_MYOP
int main(int argc, char **argv) {
    myop();
}
#endif
