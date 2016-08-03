/**
 *  \file clipboard_test.c
 *  \brief Test application for clipboard
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libclipboard.h>

int main(int argc, char *argv[]) {
    clipboard_c *cb = clipboard_new(NULL);
    if (cb == NULL) {
        printf("clipboard init failed!\n");
        return 1;
    }

    printf("Initted!\n");
    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, stdin) && strcmp(buf, "q\n")) {
        if (buf[0] == 'x') {
            clipboard_set_text(cb, buf);
            printf("SET! OWNED: %d\n", clipboard_has_ownership(cb, LC_CLIPBOARD));
            fflush(stdout);
        } else {
            char *ret = clipboard_text(cb);
            if (ret) {
                printf("OWNED: %d CB: '%s'\n", clipboard_has_ownership(cb, LC_CLIPBOARD), ret);
                fflush(stdout);
                free(ret);
            }
        }
    }

    clipboard_free(cb);
    return 0;
}
