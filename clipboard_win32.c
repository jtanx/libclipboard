/**
 *  \file clipboard_win32.c
 *  \brief Windows implementation of the clipboard.
 */

#ifdef _WIN32

#include "libclipboard.h"
#include <windows.h>

struct clipboard_c {
    CRITICAL_SECTION cs;
    DWORD last_cb_serial;
};

clipboard_c *clipboard_new(clipboard_type cb_type) {
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
    DeleteCriticalSection(&cb->cs);
    free(cb);
}

void clipboard_clear(clipboard_c *cb) {
    if (cb && OpenClipboard(NULL)) {
        EmptyClipboard();

        EnterCriticalSection(&cb->cs);
        cb->last_cb_serial = 0;
        LeaveCriticalSection(&cb->cs);

        CloseClipboard();
    }
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
            *length = len_actual - 1;
        }
    }

    GlobalUnlock(hData);
    CloseClipboard();
    return ret;
}

void clipboard_set_text(clipboard_c *cb, const char *src, int length) {
    if (cb == NULL || src == NULL) {
        return;
    }

    int max_length = strlen(src);
    if (length < 0 || length >= max_length) {
        length = max_length + 1;
    } else {
        length += 1;
    }

    int len_required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src,
                                           length, NULL, 0);
    if (len_required == 0) {
        return;
    }

    HGLOBAL buf = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * len_required);
    wchar_t *locked;
    if (buf == NULL || (locked = (wchar_t *)GlobalLock(buf)) == NULL) {
        return;
    }
    int ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, length,
                                  locked, len_required);
    if (ret == 0) {
        GlobalUnlock(buf);
        GlobalFree(buf);
        return;
    }

    /* NULL-terminate */
    locked[len_required] = 0;
    GlobalUnlock(buf);

    if (!OpenClipboard(NULL)) {
        GlobalFree(buf);
        return;
    } else if (SetClipboardData(CF_UNICODETEXT, buf) == NULL) {
        CloseClipboard();
        GlobalFree(buf);
        return;
    }

    EnterCriticalSection(&cb->cs);
    cb->last_cb_serial = GetClipboardSequenceNumber();
    LeaveCriticalSection(&cb->cs);
    CloseClipboard();
}

#endif /* _WIN32 */