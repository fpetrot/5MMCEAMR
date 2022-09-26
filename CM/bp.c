#include <stdio.h>
#include <stdlib.h>

#define ASIZE (1 << 16)
static char t[ASIZE];

int main(void)
{
#if 0
    srandom(0xdeadbeef);
    for (int i = 0; i < sizeof t / sizeof *t; i++) {
        t[i] = random()&1;
    }
    for (int i = 0; i < sizeof t / sizeof *t / 2; i++) {
        t[i] = 1;
        t[i] = random()&1;
    }
    for (int i = sizeof t / sizeof *t / 2; i < sizeof t / sizeof *t; i++) {
        t[i] = 0;
        t[i] = random()&1;
    }
#else
    for (int i = 0; i < sizeof t / sizeof *t / 2; i++) {
        t[i] = random()&1;
        t[i] = 1;
    }
    for (int i = sizeof t / sizeof *t / 2; i < sizeof t / sizeof *t; i++) {
        t[i] = random()&1;
        t[i] = 0;
    }
#endif

    int zcount = 0;
    for (int j = 0; j < 1000; j++) {
        for (int i = 0; i < sizeof t / sizeof *t; i++) {
            if (t[i] == 0) {
                zcount++;
            }
        }
    }
    printf("nb of zeros: %d\n", zcount);
    return 0;
}
