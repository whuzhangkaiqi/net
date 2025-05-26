#pragma once

#include "noncopyable.hpp"
#include <thread>
#include <functional>
#include <memory>
#include <sys/unistd.h>
#include <string>
#include <atomic>

class Thread : noncopyable
{
    public:
        using ThreadFunc = std::function<void()>;
        explicit Thread(ThreadFunc func, const std::string & name = std::string());
        ~Thread();

        void start();
        void join();

        bool started() const { return started_; }
        pid_t tid() const { return tid_; } // 这里返回的不是真正的线程号
        const std::string& name() const { return name_; }
        static int numCreated() { return numCreated_; }


    protected:
    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        std::shared_ptr<std::thread> thread_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        static std::atomic_int numCreated_;
};