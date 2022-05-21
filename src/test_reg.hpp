#pragma once
#include <list>
#include <concepts>  
#include <string>
#include <iostream>

#define TEST(name) \
    class Test_##name : public rxy::Test { \
    public: \
        Test_##name() : Test(#name) {} \
        void run() override; \
    }; \
    static rxy::TestRegister<Test_##name> dummy_##name; \
    void Test_##name::run()

#define TEST_ALL \
    rxy::TestRegistry::get_instance().run_tests(); 

namespace rxy {

class Test {
protected:
    std::string name_;
public:
    Test(const char* name) : name_(name) {}
    virtual void run() = 0;
    std::string const& name() const {
        return name_;
    }
};

class TestRegistry {
private:
    std::list<Test*> tests;
public:
    ~TestRegistry() {
        for (auto test : tests) {
            if (test) delete test;
        }
    }

    static auto& get_instance() {
        static TestRegistry instance;
        return instance;
    }

    void add_test(Test* test) {
        tests.push_back(test);
    }

    void run_tests() {
        for (auto test : tests) {
            std::cout << "Testing: " << test->name() << " ... " << std::endl;
            test->run();
        }
    }
};

template<typename T>
requires std::default_initializable<T> && std::derived_from<T, Test>
class TestRegister {
public:
    TestRegister() {
        TestRegistry::get_instance().add_test(new T());
    }
};

}