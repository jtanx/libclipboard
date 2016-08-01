/**
 *  \file clipboard_x11.c
 *  \brief X11 implementation of the clipboard.
 *
 *  Blegh I hate X11
 */

#if defined __linux__ || LIBCLIPBOARD_FORCE_X11

#include "libclipboard.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <pthread.h>

#define X_ATOM_TARGETS   0
#define X_ATOM_LENGTH    1
#define X_ATOM_MULTIPLE  2
#define X_ATOM_IDENTIFY  3
#define X_ATOM_TIMESTAMP 4
#define X_ATOM_CLIPBOARD 5
#define X_UTF8_STRING    6
#define X_ATOM_END       7

typedef union atom_c {
    xcb_atom_t atom;
    xcb_intern_atom_cookie_t cookie;
} atom_c;

/** X11 Implementation of the clipboard context **/
struct clipboard_c {
    /** XCB Display connection **/
    xcb_connection_t *xc;
    /** XCB Default screen **/
    xcb_screen_t *xs;
    /** Standard atoms **/
    atom_c std_atoms[X_ATOM_END];
    /** Our window to use for messages **/
    xcb_window_t xw;
    /** Action timeout (ms) **/
    int action_timeout;

    /** Event loop thread **/
    pthread_t event_loop;
    /** Indicates true iff event_loop is initted **/
    bool event_loop_initted;
    /** Mutex for access to context data **/
    pthread_mutex_t mu;
    /** Indicates true iff mu is initted **/
    bool mu_initted;
    /** Condition variable to notify when action is complete **/
    pthread_cond_t cond;
    /** Indicates true iff cond is initted **/
    bool cond_initted;
};

const char const *g_std_atom_names[X_ATOM_END] = {
    "TARGETS", "LENGTH", "MULTIPLE", "IDENTIFY",
    "TIMESTAMP", "CLIPBOARD", "UTF8_STRING",
};

static bool x11_intern_atoms(xcb_connection_t *xc, atom_c *atoms, const char const **atom_names, int number) {
    for (int i = 0; i < number; i++) {
        atoms[i].cookie = xcb_intern_atom(xc, 0,
                                          strlen(atom_names[i]), atom_names[i]);
    }

    for (int i  = 0; i < number; i++) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xc,
                                         atoms[i].cookie, NULL);
        if (reply == NULL) {
            return false;
        }

        atoms[i].atom = reply->atom;
        free(reply);
    }

    return true;
}

/* Based on https://xcb.freedesktop.org/xlibtoxcbtranslationguide/ */
static xcb_screen_t *x11_get_screen(xcb_connection_t *xc, int screen) {
    xcb_screen_iterator_t iter;

    iter = xcb_setup_roots_iterator(xcb_get_setup(xc));
    for (; iter.rem; screen--, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }

    return NULL;
}

static void *x11_event_loop(void *arg) {
    clipboard_c *cb = (clipboard_c *)arg;
    xcb_generic_event_t *e;

    while ((e = xcb_wait_for_event(cb->xc))) {
        switch (e->response_type & ~0x80) {
            case XCB_DESTROY_NOTIFY: {
                xcb_destroy_notify_event_t *evt = (xcb_destroy_notify_event_t *)e;
                if (evt->window == cb->xw) {
                    free(e);
                    return NULL;
                }
            }
            break;
            default: {
                /* Ignore unknown messages */
                break;
            }
        }
        free(e);
    }

    printf("x11_event_loop: [Warn] xcb_wait_for_event returned NULL\n");
    return NULL;
}

clipboard_c *clipboard_new(clipboard_opts *cb_opts) {
    clipboard_opts defaults = {
        .x11_display_name = NULL,
        .action_timeout = LC_ACTION_TIMEOUT_DEFAULT
    };

    if (cb_opts == NULL) {
        cb_opts = &defaults;
    }

    clipboard_c *cb = calloc(1, sizeof(clipboard_c));
    if (cb == NULL) {
        return NULL;
    }

    cb->action_timeout = cb_opts->action_timeout > 0 ?
                         cb_opts->action_timeout : LC_ACTION_TIMEOUT_DEFAULT;

    cb->mu_initted = pthread_mutex_init(&cb->mu, NULL) == 0;
    if (!cb->mu_initted) {
        clipboard_free(cb);
        return NULL;
    }

    cb->cond_initted = pthread_cond_init(&cb->cond, NULL) == 0;
    if (!cb->cond_initted) {
        clipboard_free(cb);
        return NULL;
    }

    int preferred_screen;
    cb->xc = xcb_connect(cb_opts->x11_display_name, &preferred_screen);
    assert(cb->xc != NULL); /* Docs say return is never NULL */
    if (xcb_connection_has_error(cb->xc) != 0) {
        clipboard_free(cb);
        return NULL;
    }
    cb->xs = x11_get_screen(cb->xc, preferred_screen);
    assert(cb->xs != NULL);

    if (!x11_intern_atoms(cb->xc, cb->std_atoms, g_std_atom_names, X_ATOM_END)) {
        clipboard_free(cb);
        return NULL;
    }

    /* Structure notify mask to get DestroyNotify messages */
    uint32_t event_mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    cb->xw = xcb_generate_id(cb->xc);
    xcb_generic_error_t *err = xcb_request_check(cb->xc,
                               xcb_create_window_checked(cb->xc,
                                       XCB_COPY_FROM_PARENT, cb->xw, cb->xs->root,
                                       0, 0, 10, 10, 0,  XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                       cb->xs->root_visual,
                                       XCB_CW_EVENT_MASK, &event_mask));
    if (err != NULL) {
        cb->xw = 0;
        /* Am I meant to free this? */
        free(err);
        clipboard_free(cb);
        return NULL;
    }

    cb->event_loop_initted = pthread_create(&cb->event_loop, NULL,
                                            x11_event_loop, (void *)cb) == 0;
    if (!cb->event_loop_initted) {
        clipboard_free(cb);
        return NULL;
    }

    return cb;
}

void clipboard_free(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }

    if (cb->event_loop_initted) {
        /* This should send a DestroyNotify message as the termination condition */
        xcb_destroy_window(cb->xc, cb->xw);
        xcb_flush(cb->xc);
        pthread_join(cb->event_loop, NULL);
    } else if (cb->xw != 0) {
        xcb_destroy_window(cb->xc, cb->xw);
    }

    if (cb->xc != NULL) {
        xcb_disconnect(cb->xc);
    }

    if (cb->cond_initted) {
        pthread_cond_destroy(&cb->cond);
    }
    if (cb->mu_initted) {
        pthread_mutex_destroy(&cb->mu);
    }
    free(cb);
}

void clipboard_clear(clipboard_c *cb, clipboard_mode mode) {
    if (cb == NULL || cb->xc == NULL) {
        return;
    }

    xcb_atom_t sel;

    switch (mode) {
        case LC_CLIPBOARD:
            sel = cb->std_atoms[X_ATOM_CLIPBOARD].atom;
            break;
        case LC_SELECTION:
            sel = XCB_ATOM_PRIMARY;
            break;
        default:
            return;
    }

    xcb_set_selection_owner(cb->xc, XCB_NONE, sel, XCB_CURRENT_TIME);
    xcb_flush(cb->xc);
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
