/**
 *  \file clipboard_win32.c
 *  \brief Windows implementation of the clipboard.
 */

#if defined _WIN32 || LIBCLIPBOARD_FORCE_WIN32

#include "libclipboard.h"
#include <windows.h>

/** Win32 Implementation of the clipboard context **/
struct clipboard_c {
    /** Implement thread (not process) safety on context access **/
    CRITICAL_SECTION cs;
    /** Serial no. of clipboard that's owned by this context **/
    DWORD last_cb_serial;
};

clipboard_c *clipboard_new(clipboard_type cb_type, clipboard_opts *cb_opts) {
    /* Win32: Only supports primary clipboard */
    if (cb_type != LIBCLIPBOARD_CLIPBOARD) {
        return NULL;
    }

    clipboard_c *ret = calloc(1, sizeof(clipboard_c));
    if (ret == NULL) {
        return NULL;
    }

    InitializeCriticalSection(&ret->cs);
    return ret;
}

void clipboard_free(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }

    DeleteCriticalSection(&cb->cs);
    free(cb);
}

void clipboard_clear(clipboard_c *cb) {
    if (cb == NULL) {
        return;
    }

    EnterCriticalSection(&cb->cs);
    if (!OpenClipboard(NULL)) {
        LeaveCriticalSection(&cb->cs);
        return;
    }

    EmptyClipboard();
    cb->last_cb_serial = 0;

    CloseClipboard();
    LeaveCriticalSection(&cb->cs);
}

bool clipboard_has_ownership(clipboard_c *cb) {
    bool ret = false;
    if (cb) {
        EnterCriticalSection(&cb->cs);
        ret = GetClipboardSequenceNumber() == cb->last_cb_serial;
        LeaveCriticalSection(&cb->cs);
    }
    return ret;
}

char *clipboard_text(clipboard_c *cb, int *length) {
    char *ret = NULL;

    if (cb == NULL || !OpenClipboard(NULL)) {
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) {
        CloseClipboard();
        return NULL;
    }

    wchar_t *pData = (wchar_t *)GlobalLock(hData);
    if (pData == NULL) {
        CloseClipboard();
        return NULL;
    }

    int len_required =
        WideCharToMultiByte(CP_UTF8, 0, pData, -1, NULL, 0, NULL, NULL);
    if (len_required != 0 && (ret = calloc(len_required, sizeof(char))) != NULL) {
        int len_actual =
            WideCharToMultiByte(CP_UTF8, 0, pData, -1, ret, len_required,
                                NULL, NULL);

        if (len_actual == 0) {
            free(ret);
            ret = NULL;
        } else if (length) {
            /* Length excluding the NULL terminator */
            *length = len_actual - 1;
        }
    }

    GlobalUnlock(hData);
    CloseClipboard();
    return ret;
}

bool clipboard_set_text(clipboard_c *cb, const char *src, int length) {
    if (cb == NULL || src == NULL || length == 0) {
        return false;
    }

    if (length > 0) {
        length += 1;
    }

    int len_required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src,
                                           length, NULL, 0);
    if (len_required == 0) {
        return false;
    }

    HGLOBAL buf = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * len_required);
    if (buf == NULL) {
        return false;
    }

    wchar_t *locked;
    if ((locked = (wchar_t *)GlobalLock(buf)) == NULL) {
        GlobalFree(buf);
        return false;
    }

    int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, length,
                                  locked, len_required);
    /* NULL-terminate */
    locked[len_required - 1] = 0;
    GlobalUnlock(buf);

    if (ret == 0 || !OpenClipboard(NULL)) {
        GlobalFree(buf);
        return false;
    } else if (SetClipboardData(CF_UNICODETEXT, buf) == NULL) {
        CloseClipboard();
        GlobalFree(buf);
        return false;
    }

    /* CloseClipboard appears to change the sequence number... */
    CloseClipboard();
    EnterCriticalSection(&cb->cs);
    cb->last_cb_serial = GetClipboardSequenceNumber();
    LeaveCriticalSection(&cb->cs);
    return true;
}

#endif /* _WIN32 || LIBCLIPBOARD_FORCE_WIN32 */
