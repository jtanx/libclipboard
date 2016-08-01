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

/** Default action timeout of 1500ms (on applicable platforms) **/
#define LC_ACTION_TIMEOUT_DEFAULT 1500

/**
 *  Determines which clipboard is used in called functions.
 */
typedef enum clipboard_mode {
    /** The primary (global) clipboard **/
    LC_CLIPBOARD,
    /** The (global) mouse selection clipboard **/
    LC_SELECTION,
} clipboard_mode;

/**
 *  Implementation specific options to be passed on instantiation.
 */
typedef struct clipboard_opts {
    /** Max time (ms) to wait for action to complete (X11 only) **/
    int action_timeout;
    /** The name of the X11 display (NULL for default - DISPLAY env. var.) **/
    const char *x11_display_name;
} clipboard_opts;

/** Opaque data structure for a clipboard context/instance **/
typedef struct clipboard_c clipboard_c;

#if !defined(_WIN32) && !defined(LIBCLIPBOARD_FORCE_WIN32) && \
    !defined(__linux__) && !defined(LIBCLIPBOARD_FORCE_X11)
#  error "Unsupported platform"
#endif

/**
 *  \brief Instantiates a new clipboard instance of the given type.
 *
 *  \param [in] cb_opts Implementation specific options (optional).
 *  \return The new clipboard instance, or NULL on failure.
 */
extern clipboard_c *clipboard_new(clipboard_opts *cb_opts);

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
 *  \param [in] mode Which clipboard to clear (platform dependent)
 */
extern void clipboard_clear(clipboard_c *cb, clipboard_mode mode);

/**
 *  \brief Determines if the clipboard is currently owned
 *
 *  \param [in] cb The clipboard to check
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return true iff the clipboard data is owned by the provided instance.
 */
extern bool clipboard_has_ownership(clipboard_c *cb, clipboard_mode mode);

/**
 *  \brief Retrieves the text currently held on the clipboard.
 *
 *  \param [in] cb The clipboard to retrieve from
 *  \param [out] length Returns the length of the retrieved data, excluding
 *                      the NULL terminator (optional).
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return A copy to the retrieved text. This must be free()'d by the user.
 *          Note that the text is encoded in UTF-8 format.
 */
extern char *clipboard_text_ex(clipboard_c *cb, int *length, clipboard_mode mode);

/**
 *  \brief Simplified version of clipboard_text_ex
 *
 *  \param [in] cb The clipboard to retrieve from
 *  \return As per clipboard_text_ex.
 *
 *  \details This function assumes LC_CLIPBOARD as the clipboard mode.
 */
extern char *clipboard_text(clipboard_c *cb);

/**
 *  \brief Sets the text for the provided clipboard.
 *
 *  \param [in] cb The clipboard to set the text.
 *  \param [in] src The UTF-8 encoded text to be set in the clipboard.
 *  \param [in] length The length of text to be set (excluding the NULL
 *                     terminator).
 *  \param [in] mode Which clipboard to clear (platform dependent)
 *  \return true iff the clipboard was set (false on error)
 *
 *  \details If the length parameter is -1, src is treated as a NULL-terminated
 *           string and its length will be determined automatically.
 */
extern bool clipboard_set_text_ex(clipboard_c *cb, const char *src, int length, clipboard_mode mode);

/**
 *  \brief Simplified version of clipboard_set_text_ex
 *
 *  \param [in] cb The clipboard to set the text.
 *  \param [in] src The UTF-8 encoded NULL terminated string to be set.
 *  \return true iff the clipboard was set (false on error)
 *
 *  \details This function assumes LC_CLIPBOARD as the clipboard mode.
 */
extern bool clipboard_set_text(clipboard_c *cb, const char *src);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCLIPBOARD_H_ */
