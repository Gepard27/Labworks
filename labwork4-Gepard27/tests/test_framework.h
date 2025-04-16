#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>

namespace TestFramework {

class TestCase {
public:
    TestCase(const std::string& name, std::function<void()> test)
        : name_(name), test_(test) {}

    void Run() {
        try {
            test_();
            std::cout << "[ OK ] " << name_ << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << name_ << ": " << e.what() << std::endl;
            success_ = false;
        }
    }

    bool Success() const { return success_; }

private:
    std::string name_;
    std::function<void()> test_;
    bool success_ = true;
};

class TestSuite {
public:
    static TestSuite& GetInstance() {
        static TestSuite instance;
        return instance;
    }

    void AddTest(const std::string& name, std::function<void()> test) {
        tests_.emplace_back(name, test);
    }

    bool RunAll() {
        bool all_success = true;
        for (auto& test : tests_) {
            test.Run();
            if (!test.Success()) {
                all_success = false;
            }
        }
        return all_success;
    }

private:
    std::vector<TestCase> tests_;
};

class AssertionFailedException : public std::exception {
public:
    AssertionFailedException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

template<typename T>
void AssertEqual(const T& expected, const T& actual, const std::string& message = "") {
    if (!(expected == actual)) {
        throw AssertionFailedException(message.empty() ? 
            "Expected equal values" : message);
    }
}

void AssertTrue(bool condition, const std::string& message = "") {
    if (!condition) {
        throw AssertionFailedException(message.empty() ? 
            "Expected true condition" : message);
    }
}

} // namespace TestFramework

#define TEST(name) \
    void Test_##name(); \
    namespace { \
        struct TestRegistrar_##name { \
            TestRegistrar_##name() { \
                TestFramework::TestSuite::GetInstance().AddTest(#name, Test_##name); \
            } \
        } test_registrar_##name; \
    } \
    void Test_##name()

#define ASSERT_TRUE(condition) \
    TestFramework::AssertTrue((condition), "Assertion failed: " #condition)

#define ASSERT_EQ(expected, actual) \
    TestFramework::AssertEqual((expected), (actual), \
        "Expected equality of " #expected " and " #actual)
