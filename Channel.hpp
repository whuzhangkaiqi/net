#pragma once

#include <functional>
#include <memory>

#include "noncopyable.hpp"
#include "Timestamp.hpp"
/**
 *  Channel 理解为通道，在muduo中，封装了socket fd（文件描述符）以及其感兴趣的事件，如EPOLLIN、EPOLLOUT等。还绑定了poller返回的具体事件
 */
class EventLoop; // 前向声明

class Channel : noncopyable
{
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        Channel(EventLoop* loop, int fd);
        ~Channel();

        // fd得到poller通知后，处理事件的函数，调用对应的回调方法。
        void handleEvent(Timestamp& receiveTime);
        void SetReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
        void SetWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
        void SetCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
        void SetErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

        // 防止当fd被关闭后，Channel还在使用
        void tie(const std::shared_ptr<void> & obj);
        int fd () const { return fd_; } // 获取fd
        int events() const { return events_; } // 获取注册的事件
        int set_revents(int revt) { revents_ = revt; return revents_; } // 设置poller返回的事件
        
        void enableReading() { events_ |= kReadEvent; update(); } // 注册读事件
        void disableReading() { events_ &= ~kReadEvent; update(); } // 取消注册读事件
        void enableWriting() { events_ |= kWriteEvent; update(); } // 注册写事件
        void disableWriting() { events_ &= ~kWriteEvent; update(); } // 取消注册写事件
        void disableAll() { events_ = kNoneEvent; update(); } // 取消注册所有事件
        
        bool isNoneEvent() const { return events_ == kNoneEvent; } // 判断是否没有事件
        bool isWriting() const { return events_ & kWriteEvent; } // 判断是否注册了写事件
        bool isReading() const { return events_ & kReadEvent; } // 判断是否注册了读事件
    
        // for poller
        int index() const { return index_; } // 获取Channel在Poller中的索引
        void set_index(int idx) { index_ = idx; } // 设置Channel在Poller中的索引

        // one loop per thread
        EventLoop* ownerLoop() { return loop_; } // 获取当前channel所属的EventLoop
        void remove();

    protected:
    private:
        void update(); // 更新poller中的事件
        void handleEventWithGuard(Timestamp& receiveTime); // 处理事件的函数，调用对应的回调方法

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_; // 事件循环
        const int fd_; // Poller监听的对象
        int events_; // 注册fd感兴趣的事件
        int revents_; // Poller返回的实际发生的事件
        int index_; // Channel在Poller中的索引

        std::weak_ptr<void> tie_; // 绑定的对象, 用来监测channel的生命周期
        bool tied_; // 是否绑定了对象

        //因为Channel中能获知fd具体发生的事件，所以它负责调用相应的回调函数
        ReadEventCallback readCallback_; // 读事件回调函数
        EventCallback writeCallback_; // 写事件回调函数
        EventCallback closeCallback_; // 关闭事件回调函数
        EventCallback errorCallback_; // 错误事件回调函数
};