#pragma once

#ifdef ESP32

#    include <src/Assert.h>

// TODO FIXME: create TEST class with a static initializer.

class TestFactory {
    TestFactory() : first(nullptr) {}
    TestFactory(const TestFactory& o) = default;

    TestBase* first;

public:
    static TestFactory& instance() {
        TestFactory instance_;
        return instance_;
    }

    void registerTest(TestBase* test) {
        test->next = first;
        first      = test;
    }

    void runAll() {
        auto current = first;
        while (current) {
            try {
                current->run();
            } catch (...) {
                // TODO FIXME: stack traces
            }
            current = current->next;
        }
    }
};

struct TestBase {
    TestBase() : next(nullptr) {}
    TestBase* next;

    virtual void run() = 0;
};

#    define TEST_CLASS_NAME(testCase, testName) testCase##_##testName##_Test

#    define TEST(testCase, testName)                                                                                                       \
        struct TEST_CLASS_NAME(testCase, testName) : TestBase {                                                                            \
            TEST_CLASS_NAME(testCase, testName)() { TestFactory::instance().registerTest(this); }                                          \
                                                                                                                                           \
            void run() override;                                                                                                           \
        };                                                                                                                                 \
                                                                                                                                           \
        TEST_CLASS_NAME(testCase, testName) instance_of_##TEST_CLASS_NAME(testCase, testName);                                             \
                                                                                                                                           \
        TEST_CLASS_NAME(testCase, testName)::run()

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

#    define TEST(test_case_name, test_name) GTEST_TEST(test_case_name, test_name)

#    define AssertThrow(statement) GTEST_TEST_ANY_THROW_(statement, GTEST_FATAL_FAILURE_)

#endif
