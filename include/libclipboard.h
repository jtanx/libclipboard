/**
*  \file  libclipboard.h
*  \brief Main header file
*/

#ifndef _LIBCLIPBOARD_H_
#define _LIBCLIPBOARD_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Determines which clipboard is used in called functions.
 */
typedef enum clipboard_type {
    /** The primary (global) clipboard **/
    LIBCLIPBOARD_CLIPBOARD,
    /** The (global) mouse selection clipboard **/
    LIBCLIPBOARD_SELECTION,
} clipboard_type;

/**
 *  Implementation specific options to be passed on instantiation.
 */
typedef struct clipboard_opts {
    /** Max time (ms) to wait for action to complete (X11 only) **/
    int action_timeout;
} clipboard_opts;

/** Opaque data structure for a clipboard context/instance **/
typedef struct clipboard_c clipboard_c;

#if !defined(_WIN32) && !defined(LIBCLIPBOARD_FORCE_WIN32)
#  error "Unsupported platform"
#endif

/**
 *  \brief Instantiates a new clipboard instance of the given type.
 *
 *  \param [in] cb_type The clipboard type to create.
 *  \param [in] cb_opts Implementation specific options (optional).
 *  \return The new clipboard instance, or NULL on failure.
 *
 *  \details Not all clipboard types are available on all platforms. In these
 *           cases, NULL is returned. For example, NULL would be returned if
 *           the selection clipboard was requested on Windows.
 */
extern clipboard_c *clipboard_new(clipboard_type cb_type, clipboard_opts *cb_opts);

/**
 *  \brief Frees associated clipboard data from the provided structure.
 *
 *  \param [in] cb The clipboard to be freed.
 */
extern void clipboard_free(clipboard_c *cb);

/**
 *  \brief Clears the contents of the given clipboard.
 *
 *  \param [in] cb The clipboard to clear.
 */
extern void clipboard_clear(clipboard_c *cb);

/**
 *  \brief Determines if the clipboard is currently owned
 *
 *  \param [in] cb The clipboard to check
 *  \return true iff the clipboard data is owned by the provided instance.
 */
extern bool clipboard_has_ownership(clipboard_c *cb);

/**
 *  \brief Retrieves the text currently held on the clipboard.
 *
 *  \param [in] cb The clipboard to retrieve from
 *  \param [out] length Returns the length of the retrieved data, excluding
 *                      the NULL terminator (optional).
 *  \return A copy to the retrieved text. This must be free()'d by the user.
 *          Note that the text is encoded in UTF-8 format.
 */
extern char *clipboard_text(clipboard_c *cb, int *length);

/**
 *  \brief Sets the text for the provided clipboard.
 *
 *  \param [in] cb The clipboard to set the text.
 *  \param [in] src The UTF-8 encoded text to be set in the clipboard.
 *  \param [in] length The length of text to be set.
 *
 *  \details If the length parameter is -1, src is treated as a NULL-terminated
 *           string and its length will be determined automatically.
 */
extern bool clipboard_set_text(clipboard_c *cb, const char *src, int length);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCLIPBOARD_H_ */