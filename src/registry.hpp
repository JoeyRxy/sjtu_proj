#pragma once
#include <list>
#include <concepts>  
#include <string>
#include <iostream>

#define RUN(name) \
    class Job_##name : public rxy::Job { \
    public: \
        Job_##name() : Job(#name) {} \
        void run() override; \
    }; \
    static rxy::JobRegister<Job_##name> dummy_##name(true); \
    void Job_##name::run()

#define RUN_OFF(name) \
    class Job_##name : public rxy::Job { \
    public: \
        Job_##name() : Job(#name) {} \
        void run() override; \
    }; \
    static rxy::JobRegister<Job_##name> dummy_##name(false); \
    void Job_##name::run()

#define RUN_ALL \
    rxy::JobRegistry::get_instance().run_tests(); 

namespace rxy {

class Job {
protected:
    std::string name_;
public:
    Job(const char* name) : name_(name) {}
    virtual void run() = 0;
    std::string const& name() const {
        return name_;
    }
};

class JobRegistry {
private:
    std::list<Job*> tests;
public:
    ~JobRegistry() {
        for (auto test : tests) {
            if (test) delete test;
        }
    }

    static auto& get_instance() {
        static JobRegistry instance;
        return instance;
    }

    void add_test(Job* test) {
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
requires std::default_initializable<T> && std::derived_from<T, Job>
class JobRegister {
public:
    JobRegister(bool regist = true) {
        if (regist) {
            JobRegistry::get_instance().add_test(new T());
        }
    }
};

}