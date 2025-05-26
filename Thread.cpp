#include "Thread.hpp"
#include "CurrentThread.hpp"
#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    :started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(name) 
{
    setDefaultName();
}
Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach(); // thread类提供了设置分离线程的方法。在linux中本质上调用的还是pthread_detach;
    }
}
void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        // 获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_(); 
    }));
    // 必须等待上面的新线程获取创建的tid值
    sem_wait(&sem);
}
void Thread::join()
{
    joined_ = true;
    thread_->join();
}
void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread %d", num);
        name_ = buf;
    }
}
