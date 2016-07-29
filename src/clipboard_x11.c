/**
 *  \file clipboard_x11.c
 *  \brief X11 implementation of the clipboard.
 */

#if defined __linux__ || LIBCLIPBOARD_FORCE_X11

#include "libclipboard.h"
#include <stdlib.h>
#include <X11/Xlib.h>

/** X11 Implementation of the clipboard context **/
struct clipboard_c {
    char c;
};

clipboard_c *clipboard_new(clipboard_opts *cb_opts) {

}

void clipboard_free(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }
}

void clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb == NULL) {
        return;
    }
}

bool clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    bool ret = false;
    if (cb) {

    }
    return ret;
}

char *clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    char *ret = NULL;

    if (cb == NULL) {
        return NULL;
	}
    return ret;
}

bool clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    if (cb == NULL || src == NULL || length == 0) {
        return false;
    }

    return false;
}

#else
typedef int x11_dummy_define;
#endif /* __linux__ || LIBCLIPBOARD_FORCE_X11 */
