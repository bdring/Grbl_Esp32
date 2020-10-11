#pragma once

#ifdef ESP32

#    include <src/Assert.h>
#    include "TestFactory.h"

#    define TEST_CLASS_NAME(testCase, testName) testCase##_##testName##_Test

// Defines a single unit test. Basically creates a small test class.

#    define Test(testCase, testName)                                                                                                       \
        struct TEST_CLASS_NAME(testCase, testName) : TestBase {                                                                            \
            TEST_CLASS_NAME(testCase, testName)() { TestFactory::instance().registerTest(this); }                                          \
                                                                                                                                           \
            const char* unitTestCase() const override { return #testCase; }                                                                \
            const char* unitTestName() const override { return #testName; }                                                                \
            void        run() override;                                                                                                    \
        };                                                                                                                                 \
                                                                                                                                           \
        TEST_CLASS_NAME(testCase, testName) TEST_CLASS_NAME(testCase, testName)##_instance;                                                \
                                                                                                                                           \
        void TEST_CLASS_NAME(testCase, testName)::run()

// after Test we get { ... }

#    define AssertThrow(statement)                                                                                                         \
        try {                                                                                                                              \
            statement;                                                                                                                     \
            Assert(false, "Expected statement to throw.");                                                                                 \
        } catch (...) {}

#else

#    include <src/Assert.h>

// Use 'Assert(...)' please.

#    define GTEST_DONT_DEFINE_TEST 1
#    define GTEST_DONT_DEFINE_ASSERT_EQ 1
#    define GTEST_DONT_DEFINE_ASSERT_NE 1
#    define GTEST_DONT_DEFINE_ASSERT_LT 1
#    define GTEST_DONT_DEFINE_ASSERT_LE 1
#    define GTEST_DONT_DEFINE_ASSERT_GE 1
#    define GTEST_DONT_DEFINE_ASSERT_GT 1
#    define GTEST_DONT_DEFINE_FAIL 1
#    define GTEST_DONT_DEFINE_SUCCEED 1

#    include "gtest/gtest.h"

#    undef EXPECT_THROW
#    undef EXPECT_NO_THROW
#    undef EXPECT_ANY_THROW
#    undef ASSERT_THROW
#    undef ASSERT_NO_THROW
#    undef ASSERT_ANY_THROW

#    define Test(test_case_name, test_name) GTEST_TEST(test_case_name, test_name)

#    define AssertThrow(statement) GTEST_TEST_ANY_THROW_(statement, GTEST_FATAL_FAILURE_)

#endif
