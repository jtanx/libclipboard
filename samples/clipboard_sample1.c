/**
 *  \file clipboard_test.c
 *  \brief Test application for clipboard
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <libclipboard.h>

int main(int argc, char *argv[]) {
    clipboard_c *cb = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    if (cb == NULL) {
        printf("clipboard init failed!\n");
        return 1;
    }

    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, stdin) && strcmp(buf, "q\n")) {
        if (buf[0] == 'x') {
            clipboard_set_text(cb, buf, -1);
            printf("SET! OWNED: %d\n", clipboard_has_ownership(cb));
            fflush(stdout);
        } else {
            char *ret = clipboard_text(cb, NULL);
            if (ret) {
                printf("OWNED: %d CB: '%s'\n", clipboard_has_ownership(cb), ret);
                fflush(stdout);
                free(ret);
            }
        }
    }
}