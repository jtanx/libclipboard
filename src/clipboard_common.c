/**
 *  \file clipboard_common.c
 *  \brief Common implementation functions of the clipboard.
 */

#include "libclipboard.h"
#include <stdlib.h>

LCB_EX char* LCB_CC clipboard_text(clipboard_c *cb) {
    return clipboard_text_ex(cb, NULL, LC_CLIPBOARD);
}

LCB_EX bool LCB_CC clipboard_set_text(clipboard_c *cb, const char *src) {
    return clipboard_set_text_ex(cb, src, -1, LC_CLIPBOARD);
}
