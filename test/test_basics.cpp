/**
 *  \file test_basics.cpp
 *  \brief Basic unit tests
 *
 *  \copyright Copyright (C) 2016 Jeremy Tan.
 *             This file is released under the MIT license.
 *             See LICENSE for details.
 */
#include <gtest/gtest.h>
#include <libclipboard.h>
#include <vector>

#include "libclipboard-test-private.h"

class BasicsTest : public ::testing::Test {
};

TEST_F(BasicsTest, TestInstantiation) {
    clipboard_c *ret = clipboard_new(NULL);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);
    // TODO(jtanx): Insert platform specific tests based on clipboard_opts
#ifdef LIBCLIPBOARD_BUILD_WIN32
    clipboard_opts opts = {0};
    ret = clipboard_new(&opts);
    ASSERT_TRUE(ret != NULL);
    clipboard_free(ret);
#endif
}

TEST_F(BasicsTest, TestMultipleInstantiation) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);

    ASSERT_TRUE(cb1 != NULL);
    ASSERT_TRUE(cb2 != NULL);

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestClipboardFreeWithNull) {
    /* Just make sure it doesn't segfault */
    clipboard_free(NULL);
}

TEST_F(BasicsTest, TestClearingClipboard) {
    clipboard_c *cb = clipboard_new(NULL);

    clipboard_set_text_ex(cb, "cleartest", -1, LCB_CLIPBOARD);
    /* Line below should have no effect */
    clipboard_clear(NULL, LCB_CLIPBOARD);

    char *text = clipboard_text_ex(cb, NULL, LCB_CLIPBOARD);
    ASSERT_STREQ("cleartest", text);
    free(text);

    clipboard_clear(cb, LCB_CLIPBOARD);
    TRY_RUN_NE(clipboard_text_ex(cb, NULL, LCB_CLIPBOARD), NULL, text);
    ASSERT_TRUE(text == NULL);

    clipboard_free(cb);
}

TEST_F(BasicsTest, TestOwnership) {
    clipboard_c *cb1 = clipboard_new(NULL);
    clipboard_c *cb2 = clipboard_new(NULL);
    ASSERT_FALSE(clipboard_has_ownership(cb1, LCB_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(cb2, LCB_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(cb1, LCB_SELECTION));
    ASSERT_FALSE(clipboard_has_ownership(cb2, LCB_SELECTION));
    ASSERT_FALSE(clipboard_has_ownership(NULL, LCB_CLIPBOARD));
    ASSERT_FALSE(clipboard_has_ownership(NULL, LCB_SELECTION));

    /* This test is inherently subject to race conditions as any other
       application could obtain the clipboard between setting and assertion. */
    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, LCB_CLIPBOARD));
    ASSERT_TRUE(clipboard_has_ownership(cb1, LCB_CLIPBOARD));

    char *ret;
    ASSERT_FALSE(clipboard_has_ownership(cb2, LCB_CLIPBOARD));
    /*
       The line below is present only for synchronisation purposes.
       On X11, it may happen that cb2's set text call happens *before*
       cb1's, meaning that the ownership would still belong to cb1.
     */
    TRY_RUN_EQ(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), NULL, ret);
    ASSERT_TRUE(ret != NULL);
    free(ret);
    ASSERT_TRUE(clipboard_set_text_ex(cb2, "test2", -1, LCB_CLIPBOARD));

    bool has_ownership;
    TRY_RUN_EQ(clipboard_has_ownership(cb1, LCB_CLIPBOARD), true, has_ownership);
    ASSERT_FALSE(has_ownership);
    ASSERT_TRUE(clipboard_has_ownership(cb2, LCB_CLIPBOARD));

    clipboard_free(cb2);
    clipboard_free(cb1);
}

TEST_F(BasicsTest, TestSetTextEdgeCases) {
    std::vector<clipboard_mode> modes{LCB_CLIPBOARD, LCB_SELECTION};
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
    char *ret1, *ret2;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", -1, LCB_CLIPBOARD));

    ret1 = clipboard_text_ex(cb1, NULL, LCB_CLIPBOARD);
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "test", ret2);
    ASSERT_STREQ("test", ret1);
    ASSERT_STREQ("test", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb2, "string", -1, LCB_CLIPBOARD));
    TRY_RUN_STRNE(clipboard_text_ex(cb1, NULL, LCB_CLIPBOARD), "string", ret1);
    ret2 = clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD);
    ASSERT_STREQ("string", ret1);
    ASSERT_STREQ("string", ret2);
    free(ret1);
    free(ret2);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "test", 1, LCB_CLIPBOARD));
    ret1 = clipboard_text_ex(cb1, NULL, LCB_CLIPBOARD);
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "t", ret2);
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

    ASSERT_TRUE(clipboard_text_ex(NULL, NULL, LCB_CLIPBOARD) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, NULL, LCB_SELECTION) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, &length, LCB_CLIPBOARD) == NULL);
    ASSERT_TRUE(clipboard_text_ex(NULL, &length, LCB_SELECTION) == NULL);

    clipboard_set_text_ex(cb1, "test", -1, LCB_CLIPBOARD);
    ret = clipboard_text_ex(cb1, NULL, LCB_CLIPBOARD);
    ASSERT_STREQ("test", ret);
    free(ret);

    ret = clipboard_text_ex(cb1, &length, LCB_CLIPBOARD);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(static_cast<int>(strlen("test")), length);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, &length, LCB_CLIPBOARD), "test", ret);
    ASSERT_STREQ("test", ret);
    ASSERT_EQ(static_cast<int>(strlen("test")), length);
    free(ret);

    clipboard_set_text_ex(cb1, "test", 2, LCB_CLIPBOARD);
    ret = clipboard_text_ex(cb1, &length, LCB_CLIPBOARD);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(static_cast<int>(strlen("te")), length);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, &length, LCB_CLIPBOARD), "te", ret);
    ASSERT_STREQ("te", ret);
    ASSERT_EQ(static_cast<int>(strlen("te")), length);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

TEST_F(BasicsTest, TestUTF8InputOutput) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "\xe6\x9c\xaa\xe6\x9d\xa5", -1, LCB_CLIPBOARD));
    ret = clipboard_text_ex(cb1, NULL, LCB_CLIPBOARD);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    ASSERT_STREQ("\xe6\x9c\xaa\xe6\x9d\xa5", ret);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}

TEST_F(BasicsTest, TestNewlines) {
    clipboard_c *cb1 = clipboard_new(NULL), *cb2 = clipboard_new(NULL);
    char *ret;

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\r\n b\r\n c\r\n", -1, LCB_CLIPBOARD));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "a\r\n b\r\n c\r\n", ret);
    ASSERT_STREQ("a\r\n b\r\n c\r\n", ret);
    free(ret);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\n b\n c\n", -1, LCB_CLIPBOARD));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "a\n b\n c\n", ret);
    ASSERT_STREQ("a\n b\n c\n", ret);
    free(ret);

    ASSERT_TRUE(clipboard_set_text_ex(cb1, "a\r b\r c\r", -1, LCB_CLIPBOARD));
    TRY_RUN_STRNE(clipboard_text_ex(cb2, NULL, LCB_CLIPBOARD), "a\r b\r c\r", ret);
    ASSERT_STREQ("a\r b\r c\r", ret);
    free(ret);

    clipboard_free(cb1);
    clipboard_free(cb2);
}
