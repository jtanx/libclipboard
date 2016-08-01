/**
 *  \file smoke_test1.c
 *  \brief Simple test of fast init/deinit to weed out race conditions
 */

#include "libclipboard.h"

#ifndef _WIN32
#  ifndef _BSD_SOURCE
#    define _BSD_SOURCE
#  endif
#  include <unistd.h>
#else
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_ITER 500

int main(int argc, char *argv[]) {
    srand(time(NULL));

    for (int i = 0; i < N_ITER; i++) {
        clipboard_c *cb = clipboard_new(NULL);
        if (cb == NULL) {
            printf("FAIL - clipboard_new returned NULL - ITER %d\n", i + 1);
            return 1;
        }
        clipboard_free(cb);
    }

    for (int i = 0; i < N_ITER / 5; i++) {
        clipboard_c *cb = clipboard_new(NULL);
        if (cb == NULL) {
            printf("FAIL - clipboard_new returned NULL - ITER %d\n", i + 1);
            return 1;
        }

#ifndef _WIN32
        usleep(rand() % 15000);
#else
        Sleep(rand() % 15);
#endif

        clipboard_free(cb);
    }
    printf("PASS\n");
    return 0;
}