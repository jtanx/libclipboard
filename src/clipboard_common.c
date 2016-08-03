/**
 *  \file clipboard_common.c
 *  \brief Common implementation functions of the clipboard.
 */

#include "libclipboard.h"
#include <stdlib.h>

char *clipboard_text(clipboard_c *cb) {
    return clipboard_text_ex(cb, NULL, LC_CLIPBOARD);
}

bool clipboard_set_text(clipboard_c *cb, const char *src) {
    return clipboard_set_text_ex(cb, src, -1, LC_CLIPBOARD);
}
