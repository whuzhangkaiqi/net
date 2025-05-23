#pragma once

#include "noncopyable.hpp"
#include "Timestamp.hpp"
#include "CurrentThread.hpp"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
// 事件循环类 主要包含了两个大模块，Channel 和 Poller
// 1. Channel 主要是对文件描述符的封装，主要包含了文件描述符的读写事件的注册和处理
// 2. Poller 主要是对事件的分发和处理，主要包含了epoll的实现

class Channel;
class Poller;

class EventLoop : noncopyable
{
    public:
        using Functor = std::function<void()>;
        EventLoop();
        ~EventLoop();

        // 开启事件循环;
        void loop();
        // 退出事件
        void quit();

        Timestamp pollReturnTime() const { return pollReturnTime_; }
        // 在当前loop中执行
        void runInLoop(Functor cb);
        // 把cb放入队列中，唤醒loop所在的线程，执行cb
        void queueInLoop(Functor cb);
        // 唤醒loop所在的线程
        void wakeup();

        // 调用Poller的对应方法
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        bool hasChannel(Channel *channel);

        // 判断eventloop的对象是否在自己的线程中
        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    protected:
    private:
        void handleRead();
        void doPendingFunctors();

        using ChannelList = std::vector<Channel *>;
        std::atomic_bool looping_; // 原子操作，通过CAS实现。
        std::atomic_bool quit_; // 标志退出loop循环
        
        const pid_t threadId_; // 记录当前loop所在线程的Id.
        
        Timestamp pollReturnTime_; // poller返回发生事件channel的时间点
        std::unique_ptr<Poller> poller_;
        
        int wakeupFd_; // 使用内核中的eventfd，是线程间的通信机制，当主loop获取一个新用户的channel，通过轮询算法唤醒一个subloop处理。还有一种实现方式是主loop和子loop都创建一个socket，通过socket_pair，网络通信。
        std::unique_ptr<Channel> wakeupChannel_;
        
        ChannelList activateChannels_;

        std::atomic_bool callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
        std::vector<Functor> pendingFunctors_; // 存储loop所有需要执行的回调操作
        std::mutex mutex_; // 互斥锁，用来保护上面vector容器的线程安全操作
};