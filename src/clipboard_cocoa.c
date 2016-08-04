/**
 *  \file clipboard_cocoa.c
 *  \brief OS X (Cocoa) implementation of the clipboard.
 */

#include "libclipboard.h"
#ifdef LIBCLIPBOARD_BUILD_COCOA

#include "libclipboard.h"
#include <stdlib.h>

#include <libkern/OSAtomic.h>
#include <Cocoa/Cocoa.h>

/** Cocoa Implementation of the clipboard context **/
struct clipboard_c {
    /** Handle to the global pasteboard. Really this doesn't need to be here... */
    NSPasteboard *pb;
    /** Pasteboard serial at last check **/
    volatile long last_cb_serial;
};

clipboard_c *clipboard_new(clipboard_opts *cb_opts) {
    clipboard_c *cb = calloc(1, sizeof(clipboard_c));
    if (cb == NULL) {
        return NULL;
    }
    cb->pb = [NSPasteboard generalPasteboard];
    return cb;
}

void clipboard_free(clipboard_c *cb) {
    free(cb);
}

void clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb != NULL) {
        [cb->pb clearContents];
    }
}

bool clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode) {
    if (cb) {
        return [cb->pb changeCount] == cb->last_cb_serial;
    }
    return false;
}

char *clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode) {
    NSString *ns_clip;
    const char *utf8_clip;
    char *ret;
    size_t len;

    if (cb == NULL) {
        return NULL;
    }

    /* OS X 10.6 and later: Should use NSPasteboardTypeString */
    ns_clip = [cb->pb stringForType:NSStringPboardType];
    if (ns_clip == nil) {
        return NULL;
    }

    utf8_clip = [ns_clip UTF8String];
    len = strlen(utf8_clip);
    ret = malloc(len + 1);
    if (ret != NULL) {
        memcpy(ret, utf8_clip, len);
        ret[len] = '\0';

        if (length) {
            *length = len;
        }
    }
    return ret;
}

bool clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode) {
    if (cb == NULL || src == NULL || length == 0) {
        return false;
    }

    NSString *ns_clip;
    bool ret;

    if (length < 0) {
        ns_clip = [[NSString alloc] initWithUTF8String:src];
    } else {
        ns_clip = [[NSString alloc] initWithBytes:src length:length encoding:NSUTF8StringEncoding];
    }

    [cb->pb declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
    ret = [cb->pb setString:ns_clip forType:NSStringPboardType];
    [ns_clip release];

    long serial = [cb->pb changeCount];
    OSAtomicCompareAndSwapLong(cb->last_cb_serial, serial, &cb->last_cb_serial);
    return ret;
}

#endif /* LIBCLIPBOARD_BUILD_COCOA */
