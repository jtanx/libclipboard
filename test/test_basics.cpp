#include <gtest/gtest.h>
#include <libclipboard.h>
#include <vector>
#include <thread>
#include <chrono>

using std::this_thread::sleep_for;
using std::chrono::milliseconds;

class BasicsTest : public ::testing::Test {
};

TEST_F(BasicsTest, TestInstantiation) {
    clipboard_c *ret = clipboard_new(NULL);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);
    // TODO(jtanx): Insert platform specific tests based on clipboard_opts
}

TEST_F(BasicsTest, TestMultipleInstantiation) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);

    ASSERT_TRUE(cb1 != NULL);
    ASSERT_TRUE(cb2 != NULL);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestClearingClipboard) {
    clipboard_c *cb = clipboard_new(NULL);
    clipboard_clear(cb, LC_CLIPBOARD);
    ASSERT_FALSE(clipboard_has_ownership(cb, LC_CLIPBOARD));

    char *text = clipboard_text_ex(cb, NULL, LC_CLIPBOARD);
    ASSERT_TRUE(text == NULL);

    clipboard_free(cb);
}

TEST_F(BasicsTest, TestOwnership) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);
    ASSERT_FALSE(clipboard_has_ownership(cb1, LC_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(cb2, LC_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(cb1, LC_SELECTION));
    ASSERT_FALSE(clipboard_has_ownership(cb2, LC_SELECTION));
    ASSERT_FALSE(clipboard_has_ownership(NULL, LC_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(NULL, LC_SELECTION));

    /* This test is inherently subject to race conditions as any other
       application could obtain the clipboard between setting and assertion. */
    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, LC_CLIPBOARD));
    ASSERT_TRUE(clipboard_has_ownership(cb1, LC_CLIPBOARD));

    ASSERT_FALSE(clipboard_has_ownership(cb2, LC_CLIPBOARD));
    ASSERT_TRUE(clipboard_set_text_ex(cb2, "test2", -1, LC_CLIPBOARD));
#ifdef __linux__
    /* Race condition on X11: SelectionClear event comes after checking for has_ownership */
    sleep_for(milliseconds(100));
#endif
    ASSERT_FALSE(clipboard_has_ownership(cb1, LC_CLIPBOARD));
    ASSERT_TRUE(clipboard_has_ownership(cb2, LC_CLIPBOARD));

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestSetTextEdgeCases) {
    std::vector<clipboard_mode> modes{LC_CLIPBOARD, LC_SELECTION};
    clipboard_c *cb1 = clipboard_new(NULL);

    for (const auto &m : modes) {
        ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, 0, m));
        ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, -1, m));
        ASSERT_FALSE(clipboard_set_text_ex(NULL, NULL, 10, m));
        ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, 0, m));
        ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, -1, m));
        ASSERT_FALSE(clipboard_set_text_ex(cb1, NULL, 10, m));
        ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", 0, m));
        ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", -1, m));
        ASSERT_FALSE(clipboard_set_text_ex(NULL, "test", 10, m));
        ASSERT_FALSE(clipboard_set_text_ex(cb1, "test", 0, m));
    }

    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestSetText) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, LC_CLIPBOARD));

    char *ret1 = clipboard_text_ex(cb1, NULL, LC_CLIPBOARD), *ret2 = clipboard_text_ex(cb2, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("test", ret1);
    ASSERT_STREQ("test", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb2, "string", -1, LC_CLIPBOARD));
#ifdef __linux__
    /* Race condition on X11: SelectionClear event comes after checking for has_ownership */
    sleep_for(milliseconds(100));
#endif
    ret1 = clipboard_text_ex(cb1, NULL, LC_CLIPBOARD);
    ret2 = clipboard_text_ex(cb2, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("string", ret1);
    ASSERT_STREQ("string", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", 1, LC_CLIPBOARD));
#ifdef __linux__
    /* Race condition on X11: SelectionClear event comes after checking for has_ownership */
    sleep_for(milliseconds(100));
#endif
    ret1 = clipboard_text_ex(cb1, NULL, LC_CLIPBOARD);
    ret2 = clipboard_text_ex(cb2, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("t", ret1);
    ASSERT_STREQ("t", ret2);
    free(ret1);
    free(ret2);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestGetText) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;
    int length;

    ASSERT_TRUE(clipboard_text_ex(NULL, NULL, LC_CLIPBOARD) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, NULL, LC_SELECTION) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, &length, LC_CLIPBOARD) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, &length, LC_SELECTION) == NULL);

    clipboard_set_text_ex(cb1, "test", -1, LC_CLIPBOARD);
    ret = clipboard_text_ex(cb1, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("test", ret);
    free(ret);

    ret = clipboard_text_ex(cb1, &length, LC_CLIPBOARD);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(static_cast<int>(strlen("test")), length);
    free(ret);

    ret = clipboard_text_ex(cb2, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("test", ret);
    free(ret);

    clipboard_set_text_ex(cb1, "test", 2, LC_CLIPBOARD);
    ret = clipboard_text_ex(cb1, &length, LC_CLIPBOARD);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(static_cast<int>(strlen("te")), length);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

TEST_F(BasicsTest, TestUTF8InputOutput) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "\xe6\x9c\xaa\xe6\x9d\xa5", -1, LC_CLIPBOARD));
    ret = clipboard_text_ex(cb1, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    ret = clipboard_text_ex(cb2, NULL, LC_CLIPBOARD);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}
