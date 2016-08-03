/**
 *  \file clipboard_cocoa.c
 *  \brief OS X (Cocoa) implementation of the clipboard.
 */

#if defined __APPLE__ || LIBCLIPBOARD_FORCE_COCOA

#include "libclipboard.h"
#include <stdlib.h>

clipboard_c *clipboard_new(clipboard_opts *cb_opts) {
    return NULL;
}

void clipboard_free(clipboard_c *cb) {

}

void clipboard_clear(clipboard_c *cb, clipboard_mode mode) {

}

bool clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    return false;
}

char *clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    return NULL;
}

bool clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    return false;
}

#else
typedef int cocoa_dummy_define;
#endif /* __APPLE__ || LIBCLIPBOARD_FORCE_COCOA */
