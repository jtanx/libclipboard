#include <gtest/gtest.h>
#include <libclipboard.h>

class BasicsTest : public ::testing::Test {
};

TEST_F(BasicsTest, TestInstantiation) {
    clipboard_c *ret = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);

    ret = clipboard_new(LIBCLIPBOARD_SELECTION, NULL);
#if !defined(__linux__) || defined(LIBCLIPBOARD_FORCE_WIN32) || defined(LIBCLIPBOARD_FORCE_OSX)
    ASSERT_TRUE(ret == NULL);
#else
    ASSERT_TRUE(ret != NULL);
#endif
    // This should have no effect if ret is NULL.
    clipboard_free(ret);
}

TEST_F(BasicsTest, TestMultipleInstantiation) {
    clipboard_c *cb1 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    clipboard_c *cb2 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);

    ASSERT_TRUE(cb1 != NULL);
    ASSERT_TRUE(cb2 != NULL);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestClearingClipboard) {
    clipboard_c *cb = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    clipboard_clear(cb);
    ASSERT_FALSE(clipboard_has_ownership(cb));

    char *text = clipboard_text(cb, NULL);
    ASSERT_TRUE(text == NULL);

    clipboard_free(cb);
}

TEST_F(BasicsTest, TestOwnership) {
    clipboard_c *cb1 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    clipboard_c *cb2 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    ASSERT_FALSE(clipboard_has_ownership(cb1));
    ASSERT_FALSE(clipboard_has_ownership(cb2));
    ASSERT_FALSE(clipboard_has_ownership(NULL));

    /* This test is inherently subject to race conditions as any other
       application could obtain the clipboard between setting and assertion. */
    ASSERT_TRUE(clipboard_set_text(cb1, "test", -1));
    ASSERT_TRUE(clipboard_has_ownership(cb1));

    ASSERT_FALSE(clipboard_has_ownership(cb2));
    ASSERT_TRUE(clipboard_set_text(cb2, "test2", -1));
    ASSERT_FALSE(clipboard_has_ownership(cb1));
    ASSERT_TRUE(clipboard_has_ownership(cb2));

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestSetTextEdgeCases) {
    clipboard_c *cb1 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);

    ASSERT_FALSE(clipboard_set_text(NULL, NULL, 0));
    ASSERT_FALSE(clipboard_set_text(NULL, NULL, -1));
    ASSERT_FALSE(clipboard_set_text(NULL, NULL, 10));
    ASSERT_FALSE(clipboard_set_text(cb1, NULL, 0));
    ASSERT_FALSE(clipboard_set_text(cb1, NULL, -1));
    ASSERT_FALSE(clipboard_set_text(cb1, NULL, 10));
    ASSERT_FALSE(clipboard_set_text(NULL, "test", 0));
    ASSERT_FALSE(clipboard_set_text(NULL, "test", -1));
    ASSERT_FALSE(clipboard_set_text(NULL, "test", 10));
    ASSERT_FALSE(clipboard_set_text(cb1, "test", 0));

    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestSetText) {
    clipboard_c *cb1 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    clipboard_c *cb2 = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);

    ASSERT_TRUE(clipboard_set_text(cb1, "test", -1));

    char *ret1 = clipboard_text(cb1, NULL), *ret2 = clipboard_text(cb2, NULL);
    ASSERT_STREQ("test", ret1);
    ASSERT_STREQ("test", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text(cb2, "string", -1));
    ret1 = clipboard_text(cb1, NULL);
    ret2 = clipboard_text(cb2, NULL);
    ASSERT_STREQ("string", ret1);
    ASSERT_STREQ("string", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text(cb1, "test", 1));
    ret1 = clipboard_text(cb1, NULL);
    ret2 = clipboard_text(cb2, NULL);
    ASSERT_STREQ("t", ret1);
    ASSERT_STREQ("t", ret2);
    free(ret1);
    free(ret2);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestGetText) {
    clipboard_c *cb = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    char *ret;
    int length;

    ASSERT_TRUE(clipboard_text(NULL, NULL) == NULL);
    ASSERT_TRUE(clipboard_text(NULL, &length) == NULL);

    clipboard_set_text(cb, "test", -1);
    ret = clipboard_text(cb, NULL);
    ASSERT_STREQ("test", ret);
    free(ret);

    ret = clipboard_text(cb, &length);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(strlen("test"), length);
    free(ret);

    clipboard_set_text(cb, "test", 2);
    ret = clipboard_text(cb, &length);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(strlen("te"), length);
    free(ret);

    clipboard_free(cb);
}

TEST_F(BasicsTest, TestUTF8InputOutput) {
    clipboard_c *cb = clipboard_new(LIBCLIPBOARD_CLIPBOARD, NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text(cb, "\xe6\x9c\xaa\xe6\x9d\xa5", -1));
    ret = clipboard_text(cb, NULL);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    clipboard_free(cb);

}